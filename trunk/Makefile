#wsdl2h -o **** -c Client_1.wsdl Client_2.wsdl Client_3.wsdl
#soapcpp2 -c -C ****.h
#gcc -o **** ****.c soapC.c soapClient.c stdsoap2.c

WsdlFiles:=Client.wsdl
SoapLibs:=soapC.c soapClient.c stdsoap2.c
objects:=Server.o $(SoapLibs:.c=.o)
obj:=Listen.c $(SoapLibs)
server:$(objects)
	cc -o server $(objects)

Server.o:Server.h soapH.h
Listen.o:Listen.c
	cc -c -o Listen.o Listen.c
soapClient.o soapC.o stdsoap2.o:stdsoap2.h
soapClient.c soapH.h soapC.c:Server.h
	soapcpp2 -c -C Server.h
soapC.o stdsoap2.o:stdsoap2.h
Server.h:$(WsdlFiles)
	wsdl2h -o Server.h -c $(WsdlFiles)
.PHONY:listen
listen:
	cc -o listen $(obj) -l mysqlclient
.PHONY:clean
clean:
	-rm server listen $(objects) *.xml *.nsmap soapStub.h soapClientLib.c Server.h soapH.h soapC.c soapClient.c

