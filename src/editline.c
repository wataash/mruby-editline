#include <stdio.h>

#include <histedit.h>

#include "mruby.h"
#include "mruby/array.h"
#include "mruby/class.h"
#include "mruby/data.h"
#include "mruby/error.h"
#include "mruby/string.h"
#include "mruby/value.h"
#include "mruby/variable.h"

struct mrb_editline {
  mrb_state *mrb;
  mrb_value self;
  EditLine *e;
  History *h;
};

static void
mrb_editline_free(mrb_state *mrb, void *ptr)
{
  struct mrb_editline *mel = ptr;

  mel->mrb = NULL;
  mel->self = mrb_nil_value();

  if (mel->e != NULL) {
    el_end(mel->e);
    mel->e = NULL;
  }

  if (mel->h != NULL) {
    history_end(mel->h);
    mel->h = NULL;
  }

  mrb_free(mrb, ptr);
}

const static struct mrb_data_type mrb_editline_type = { "EditLine", mrb_editline_free };

static unsigned char userfunction_callback(EditLine *e, int ch, int no);

#define DEFINECALLBACK(i) \
  static unsigned char callback##i (EditLine *e, int ch) \
  { return userfunction_callback(e, ch, (i)); }

DEFINECALLBACK(0)
DEFINECALLBACK(1)
DEFINECALLBACK(2)
DEFINECALLBACK(3)
DEFINECALLBACK(4)
DEFINECALLBACK(5)
DEFINECALLBACK(6)
DEFINECALLBACK(7)
DEFINECALLBACK(8)
DEFINECALLBACK(9)
#define USERFUNCTIONS 10

typedef unsigned char (*callback_t)(EditLine *, int);
const static callback_t userfunction_callback_list[] = {
  callback0, callback1, callback2, callback3, callback4,
  callback5, callback6, callback7, callback8, callback9
};

static unsigned char
userfunction_callback(EditLine *e, int ch, int no)
{
  mrb_value ary, proc, ret, tuple;
  struct mrb_editline *mel;

  el_get(e, EL_CLIENTDATA, &mel);
  ary = mrb_iv_get(mel->mrb, mel->self, mrb_intern_lit(mel->mrb, "userfunctions"));
  tuple = mrb_ary_ref(mel->mrb, ary, no);
  if (mrb_nil_p(tuple)) {
    return CC_ERROR;
  }
  proc = mrb_ary_ref(mel->mrb, tuple, 2);
  if (mrb_nil_p(proc)) {
    return CC_ERROR;
  }
  ret = mrb_funcall(mel->mrb, proc, "call", 2, mel->self, mrb_fixnum_value(ch));
  return mrb_fixnum(ret);
}

mrb_value
mrb_editline_init(mrb_state *mrb, mrb_value self)
{
  struct mrb_editline *mel;
  HistEvent hev;
  mrb_value ary;

  mel = (struct mrb_editline *)mrb_malloc(mrb, sizeof(struct mrb_editline));
  mel->mrb = mrb;
  mel->self = self;
  mel->e = el_init("mruby", stdin, stdout, stderr);
  mel->h = NULL;
  DATA_TYPE(self) = &mrb_editline_type;
  DATA_PTR(self) = mel;

  el_set(mel->e, EL_CLIENTDATA, mel);
  el_set(mel->e, EL_EDITOR, "emacs");

  mel->h = history_init();
  if (mel->h == NULL) {
    mrb_raise(mrb, E_RUNTIME_ERROR, "history_init() failed");
  }
  history(mel->h, &hev, H_SETSIZE, 100);
  el_set(mel->e, EL_HIST, history, mel->h);

  ary = mrb_ary_new_capa(mrb, USERFUNCTIONS);
  mrb_iv_set(mrb, self, mrb_intern_lit(mel->mrb, "userfunctions"), ary);
  return self;
}

mrb_value
mrb_editline_deletestr(mrb_state *mrb, mrb_value self)
{
  struct mrb_editline *mel;
  mrb_int count;

  mel = DATA_PTR(self);
  mrb_get_args(mrb, "i", &count);
  el_deletestr(mel->e, count);
  return mrb_nil_value();
}

mrb_value
mrb_editline_gets(mrb_state *mrb, mrb_value self)
{
  HistEvent hev;
  struct mrb_editline *mel;
  int count;
  const char *line;

  mel = DATA_PTR(self);
  line = el_gets(mel->e, &count);
  if (line == NULL) {
    if (count == -1) {
      mrb_sys_fail(mrb, "el_gets");
    } else {
      return mrb_nil_value();
    }
  }
  history(mel->h, &hev, H_ENTER, line);
  return mrb_str_new_cstr(mrb, line);
}

mrb_value
mrb_editline_insertstr(mrb_state *mrb, mrb_value self)
{
  struct mrb_editline *mel;
  mrb_int ret;
  mrb_value str;

  mel = DATA_PTR(self);
  mrb_get_args(mrb, "S", &str);
  ret = el_insertstr(mel->e, mrb_str_to_cstr(mrb, str));
  return mrb_fixnum_value(ret);
}

mrb_value
mrb_editline_line(mrb_state *mrb, mrb_value self)
{
  struct mrb_editline *mel;
  const LineInfo *li;
  mrb_value ary;

  mel = DATA_PTR(self);
  li = el_line(mel->e);
  ary = mrb_ary_new_capa(mrb, 2);
  mrb_ary_push(mrb, ary, mrb_str_new(mel->mrb, li->buffer, li->lastchar - li->buffer));
  mrb_ary_push(mrb, ary, mrb_fixnum_value(li->cursor - li->buffer));
  return ary;
}

mrb_value
mrb_editline_push(mrb_state *mrb, mrb_value self)
{
  struct mrb_editline *mel;
  char *str;

  mel = DATA_PTR(self);
  mrb_get_args(mrb, "z", &str);
  el_push(mel->e, str);
  return mrb_nil_value();
}

static char *
cb_prompt(EditLine *e)
{
  struct mrb_editline *mel;
  mrb_value s;

  el_get(e, EL_CLIENTDATA, &mel);
  s = mrb_iv_get(mel->mrb, mel->self, mrb_intern_lit(mel->mrb, "prompt"));
  return mrb_str_to_cstr(mel->mrb, s);
}

mrb_value
mrb_editline_set_addfn(mrb_state *mrb, mrb_value self)
{
  mrb_value ary, block, name, help, tri;
  mrb_int helplen, idx, namelen;
  struct mrb_editline *mel;
  char *nameptr, *helpptr;

  ary = mrb_iv_get(mrb, self, mrb_intern_lit(mrb, "userfunctions"));
  idx = RARRAY_LEN(ary);
  if (idx == USERFUNCTIONS) {
    mrb_raise(mrb, E_ARGUMENT_ERROR, "too many user-defined functions");
  }

  mrb_get_args(mrb, "ss&", &nameptr, &namelen, &helpptr, &helplen, &block);

  name = mrb_str_new(mrb, nameptr, namelen);
  help = mrb_str_new(mrb, helpptr, helplen);

  tri = mrb_ary_new_capa(mrb, 3);
  mrb_ary_push(mrb, tri, name);
  mrb_ary_push(mrb, tri, help);
  mrb_ary_push(mrb, tri, block);

  mel = DATA_PTR(self);
  if (el_set(mel->e, EL_ADDFN, RSTRING_PTR(name), RSTRING_PTR(help), userfunction_callback_list[idx]) != 0) {
    mrb_raise(mrb, E_ARGUMENT_ERROR, "el_set(EL_ADDFN) failed");
  }
  mrb_ary_set(mrb, ary, idx, tri);

  return mrb_true_value();
}

mrb_value
mrb_editline_set_bind(mrb_state *mrb, mrb_value self)
{
  struct mrb_editline *mel;
  mrb_value key, command;
  int ret;
  char *c_key, *c_command;

  mrb_get_args(mrb, "SS", &key, &command);
  c_key = mrb_str_to_cstr(mrb, key);
  c_command = mrb_str_to_cstr(mrb, command);

  mel = DATA_PTR(self);
  ret = el_set(mel->e, EL_BIND, c_key, c_command, NULL);
  return mrb_fixnum_value(ret);
}

mrb_value
mrb_editline_set_prompt(mrb_state *mrb, mrb_value self)
{
  struct mrb_editline *mel;
  mrb_value esc, str;

  mel = DATA_PTR(self);
  esc = mrb_nil_value();
  mrb_get_args(mrb, "S|S", &str, &esc);
  mrb_iv_set(mrb, self, mrb_intern_lit(mrb, "prompt"), str);
  mrb_iv_set(mrb, self, mrb_intern_lit(mrb, "prompt_esc"), esc);
  if (mrb_nil_p(esc)) {
    el_set(mel->e, EL_PROMPT, cb_prompt);
  } else {
    el_set(mel->e, EL_PROMPT_ESC, cb_prompt, RSTRING_PTR(esc)[0]);
  }
  return self;
}

void
mrb_mruby_editline_gem_init(mrb_state *mrb)
{
  struct RClass *cls;

  cls = mrb_define_class(mrb, "EditLine", mrb->object_class);
  MRB_SET_INSTANCE_TT(cls, MRB_TT_DATA);
  mrb_define_method(mrb, cls, "deletestr", mrb_editline_deletestr, MRB_ARGS_REQ(1));
  mrb_define_method(mrb, cls, "gets", mrb_editline_gets, MRB_ARGS_NONE());
  mrb_define_method(mrb, cls, "initialize", mrb_editline_init, MRB_ARGS_NONE());
  mrb_define_method(mrb, cls, "insertstr", mrb_editline_insertstr, MRB_ARGS_REQ(1));
  mrb_define_method(mrb, cls, "line", mrb_editline_line, MRB_ARGS_REQ(0));
  mrb_define_method(mrb, cls, "push", mrb_editline_push, MRB_ARGS_REQ(1));
  mrb_define_method(mrb, cls, "set_addfn", mrb_editline_set_addfn, MRB_ARGS_REQ(3));
  mrb_define_method(mrb, cls, "set_bind", mrb_editline_set_bind, MRB_ARGS_REQ(2));
  mrb_define_method(mrb, cls, "set_prompt", mrb_editline_set_prompt, MRB_ARGS_ARG(1, 1));

#define define_el_const(SYM) mrb_define_const(mrb, cls, #SYM, mrb_fixnum_value(SYM));
  define_el_const(CC_NORM);
  define_el_const(CC_NEWLINE);
  define_el_const(CC_EOF);
  define_el_const(CC_ARGHACK);
  define_el_const(CC_REFRESH);
  define_el_const(CC_REFRESH_BEEP);
  define_el_const(CC_CURSOR);
  define_el_const(CC_REDISPLAY);
  define_el_const(CC_ERROR);
  define_el_const(CC_FATAL);
}

void
mrb_mruby_editline_gem_final(mrb_state *mrb)
{
}
