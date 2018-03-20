class HistoryError < StandardError
  def initialize(num, str)
    @num = num
    @str = str
    super(@str)
  end

  attr_reader :num, :str
end
