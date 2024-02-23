# PicowWebServer
In the picoW documentation, we find explanations for creating a web server using micropython.

But for the C language there are only examples of TCP/IP client and server to receive and send a simple random data file. 

Using the tcp_server example, I managed to write a fairly simple web server which displays an HTML form and which in return retrieves the information from the form.

After hours of work, dozens of tests and numerous internet searches on the use of the lwip library and tcp functions, the program works correctly. The main problem came from the tcp_write function which I thought needed to indicate the actual size of the data to be written. 

In fact, you must first initialize the buffer to be written with spaces then fill it with the data to write then pass it to the tcp_write function indicating the total length of the buffer therefore the BUF_SIZE value and not the real length.

The program uses the main functions of the example for all the functions of creating the wifi connection, opening the server and waiting for the client connection. 

The data writing function is suitable for sending an HTML header then the form data. 

The callback function is suitable for eliminating unnecessary requests and for decoding transmitted commands. 

Each time this function is called, the form is returned again complete with a message containing the response. 

The analysis function can of course be adapted according to the desired commands. An example of button pressing is provided in the program.

The main function only contains the calls to open the wifi connection and create the web server then a simple loop which calls the pooling function. 

It is this which will trigger the writing and reading functions through the lwip tcp engines.

Nothing is planned to stop this loop, it remains to be adapted.

#Note: For testing purposes, and to enable control messages to be displayed, this is called the USB port management procedure. 

The web server will therefore only start if a telnet or putty type connection is opened. 

For autonomous operation, it is therefore necessary to comment on this instruction.   
