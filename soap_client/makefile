INCLUDE_DIR = -I /usr/local/include/glib-2.0/ \
              -I /usr/local/lib/glib-2.0/include/ \
              -I /home/guojf/gsoap/include/  \

LD_FLAGS = -lglib-2.0 \
#STATIC = /home/guojf/gsoap/lib/libgsoap.a 
cc = gcc
TARGET1 += soap_client.c soapC.c soapClient.c
TARGET2 : soap_client.c
	$(cc) $(TARGET1) -o soap_client $(INCLUDE_DIR) /home/guojf/gsoap/lib/libgsoap.a  $(LD_FLAGS)
clean:
	rm soap_client

