# guess converted to
# build/host/mrbgems/mruby-editline/gem_init.c
# gem_mrblib_irep_mruby_editline

class HistoryError < StandardError
  def initialize(num, str)
    @num = num
    @str = str
    super(@str)
  end

  attr_reader :num, :str
end
