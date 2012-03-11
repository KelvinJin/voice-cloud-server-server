wsdl2h -o **** -c Client_1.wsdl Client_2.wsdl Client_3.wsdl
soapcpp2 -c -C ****.h
gcc -o **** ****.c soapC.c soapClient.c stdsoap2.c

