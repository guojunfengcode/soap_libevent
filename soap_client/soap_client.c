#include "soapH.h"
#include "calc.nsmap"
#include <math.h>
#include <glib.h>
/* the Web service endpoint URL */
const char server[] = "http://localhost:50000";

void display(void)
{
    g_printf("-----------welcome library management--------------\n");
    g_printf("Please show details by the following serial number\n");
    g_printf("1.book name\t2.book kind\t3.book price\t4.book time\n");
    g_printf("Or enter the following keywords to get features\n");
    g_printf("add\tdel\tmod\tinq\n");
    g_printf("----------------------------------------------------\n");
}

int main(int argc, char **argv)
{
    struct soap *soap = soap_new1(SOAP_XML_INDENT); /* new context */
    get_book new_book;
    GList **book_info;
    gchar *buf[2048];
    gchar get_num[10];
    gchar bookname[10];
    gchar bookkind[10];
    gchar bookprice[10];
    gchar modname[10];
    gchar modlatername[10];
    gchar delname[10];
    gint modkey;
    
    
    new_book.name = "cena";
    new_book.kind = "1";
    new_book.price = "3.3";
    new_book.time = "1993"; 
    display();
    while (1) {
	g_printf("=================================\n");
        g_printf("\nPlease enter the characters corresponding to the function  :");
        scanf("%s", get_num);
        
        if (strcmp(get_num, "inq") == 0) {
	    if (soap_call_ns2__inq(soap, server, "", (char **)buf) == SOAP_OK) {
                printf("OK\n");
                char *bookinfo = *buf;
		printf("====================================\n");
                printf("%s\n", bookinfo);
		printf("====================================\n");
            } else {
                soap_print_fault(soap, stderr);

            }
	} else if (strcmp(get_num, "add") == 0) {
	    g_printf("input book name  :");
            scanf("%s", bookname);
            g_printf("1.小说类[fiction]\t2.生活类[life]\t3.科普类[science]\t4.体育类[sport]\ninput book kind num :");
	    scanf("%s", bookkind);
            g_printf("input book price :");
            scanf("%s", bookprice);
            if (soap_call_ns2__add(soap, server, "", bookname, bookkind, bookprice, (char **)buf) == SOAP_OK) {
                printf("OK\n");
                char *bookinfo = *buf;
		printf("====================================\n");
                printf("%s\n", bookinfo);
		printf("====================================\n");
            } else {
                soap_print_fault(soap, stderr);  
            }
	} else if (strcmp(get_num, "del") == 0) {
	    g_printf("Ready to delete name:  ");
            scanf("%s", delname);
            if (soap_call_ns2__del(soap, server, "", delname, (char **)buf) == SOAP_OK) {
                printf("OK\n");
                char *bookinfo = *buf;
		printf("====================================\n");
                printf("%s\n", bookinfo);
		printf("====================================\n");
            } else {
                soap_print_fault(soap, stderr);
            }
	} else if (strcmp(get_num, "mod") == 0) {
	    g_printf("Please enter the item key value you want to modify :");
            scanf("%d", &modkey);
            g_printf("Please enter the book name you want to modify :");
            scanf("%s", modname);
	    g_printf("Please enter the information to be modified :");
            scanf("%s", modlatername);
	    if (soap_call_ns2__mod(soap, server, "", modkey, modname, modlatername, (char **)buf) == SOAP_OK) {
                printf("OK\n");
                char *bookinfo = *buf;
		printf("====================================\n");
                printf("%s\n", bookinfo);
		printf("====================================\n");
            } else {
                soap_print_fault(soap, stderr);
            }
	}

    }
  

    soap_destroy(soap); /* delete deserialized objects */
    soap_end(soap);     /* delete heap and temp data */
    soap_free(soap);    /* we're done with the context */
    return 0;
}

