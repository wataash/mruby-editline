# class EditLine
#   def p_prompt
#     p @userfunctions
#   end
# end

# el = EditLine.new
# # el.p_prompt  #=> nil
# el.set_prompt "prompt> "
# # el.p_prompt  #=> nil


#-----------------------------------------------------------------------
# https://github.com/iij/mruby-editline

if Object.const_defined?(:RUBY_PLATFORM)
  require "editline"
end

el = EditLine.new
el.set_prompt "prompt> "

$el = el
el.set_addfn("mruby-editline-sample-complete", "complete a word") do |ch|
  $el.insertstr "hello!" # mruby bug
  EditLine::CC_REFRESH
end
el.set_bind("^I", "mruby-editline-sample-complete")

while line = el.gets
  puts "cmd =>" + line
end


# --------------------------------------------------
#set_bind(key, proc) should be set_bind(key, name)


