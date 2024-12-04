For compiling the C file use:
gcc -o client bkchain_request_client.c -lcurl -ljson-c

For running the flask server in Debug mode, change last line to:
app.run(debug=True, port=8000)