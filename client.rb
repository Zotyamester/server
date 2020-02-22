require 'socket'

s = TCPSocket.new 'localhost', 6969

s.write("/home/asd.c\n")

s.each_line do |line|
    puts line
end

s.close
