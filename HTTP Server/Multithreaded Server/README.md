#Assignment 4 directory

This directory contains source code and other files for Assignment 4.

Use this README document to store notes about design, testing, and
questions you have while developing your assignment.

#Example 1

GET,/new.txt,200,(null)
GET,/doesntexist.txt,404,(null)
PUT,/doesntexist.txt,201,(null)
GET,/doesntexist.txt,200,(null)

curl http://localhost:1234/new.txt -o a.txt
curl http://localhost:1234/doesntexist.txt -o new.txt
curl http://localhost:1234/doesntexist.txt -T new.txt
curl http://localhost:1234/doesntexist.txt -o new.txt