# How to run server

* By Typing  ./http_server <serve_dir> <port>, server side is now waiting for the connection of client side.
![캡처](https://user-images.githubusercontent.com/78118588/210287319-a892effb-ac8f-436d-ba72-e48063e7ec1a.JPG)

# Testing Server

* By spliting up terminal for client side, and if you want to print the contents directly to screen, use CURL command.

![캡처](https://user-images.githubusercontent.com/78118588/210287446-b401ed7c-9c11-4d36-bf3d-1ad03eeb3e68.JPG)

* You can also use curl to view the HTTP headers that are received by a client from your server using the verbose (-v) option:

![캡처](https://user-images.githubusercontent.com/78118588/210287501-a014faba-345a-4cbd-b8a8-07e91be2c8a9.JPG)

* To save the body of an HTTP response to a file, use wget. For example with a server bound to port 8000 and to request the resource /quote.txt:

![캡처](https://user-images.githubusercontent.com/78118588/210287618-07c0e657-b633-4937-a264-72ac60170f3e.JPG)

## Obtain Picture
* To obtain picture and check HTTP server is working correctly, by changing quote.txt to ocelot.jpg, and type localhost:8000/ocelot.jpg,

![캡처](https://user-images.githubusercontent.com/78118588/210287748-716b8fa1-7451-4f1f-8aa2-62e4fdebc234.JPG)

Cat picture is successfully popped up.

## HTTP Transaction

![캡처](https://user-images.githubusercontent.com/78118588/210288026-f008380c-c033-4e80-ba67-13972175018b.JPG)

* Brief picture that helps to visualize concept of HTTP Request and HTTP Response.


