# mruby-editline
mruby binding for [editline(3)](http://netbsd.gw.com/cgi-bin/man-cgi?editline++NetBSD-current) library.

## Usage
```rb
el = EditLine.new
el.set_prompt "prompt> "

el.set_addfn("mruby-editline-sample-complete", "complete a word") do |ch|
  el.insertstr "hello!"
  EditLine::CC_REFRESH
end
el.set_bind("^I", "mruby-editline-sample-complete")

while line = el.gets
  puts "cmd =>" + line
end
```

## API Reference
### EditLine class
- `.new`
  - Create a new EditLine instance. (`el_init`)
- `#gets -> String|nil`
  - Read a line from the tty.  Returns the line read if successful, returns
    nil if no characters were read, or raises a RuntimeError if an error
    occurred. (`el_gets`)
- `#insertstr(str) -> Fixnum`
  - Insert str into the line at the cursor.  Returns -1 if str is empty or
    won't fit, and 0 otherwise. (`el_insertstr`)
- `#line -> [String, Fixnum]`
  - Return the current line and the position of cursor. (`el_line`)
- `#set_addfn(name, help, &proc)`
  - add an user defined function `name`.  `help` is a description of it.
    `proc` is a callback function.  The return value of `proc` should be
    one of EditLine::CC_nnn. (`el_set(EL_ADDFN)`)
  - Caveats: You cannot define more than 10 functions.  If you need more,
    change lines around the definition of `USERFUNCTIONS` in src/editline.c.
- `#set_bind(key, proc)`
  - `proc` can be String or Proc. (`el_set(EL_BIND)`)
- `#set_prompt(proc, c=nil)`
  - Define prompt printing function as `proc`, which is to return a string
    that contains the prompt. (`el_set(EL_PROMPT)`, `el_set(EL_PROMPT_ESC)`)
    - Note: ANSI escape sequence in prompt string does not work as expected
      due to a bug in editline library.

(Descriptions are from [editline(3) manual page](http://netbsd.gw.com/cgi-bin/man-cgi?editline++NetBSD-current) library.)


## License
Copyright (c) 2017 Internet Initiative Japan Inc.

Permission is hereby granted, free of charge, to any person obtaining a 
copy of this software and associated documentation files (the "Software"), 
to deal in the Software without restriction, including without limitation 
the rights to use, copy, modify, merge, publish, distribute, sublicense, 
and/or sell copies of the Software, and to permit persons to whom the 
Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in 
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR 
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, 
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE 
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER 
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING 
FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER 
DEALINGS IN THE SOFTWARE.
