#include <stdio.h>
#include <string.h>
#include <event.h>
#include <unistd.h>
#include <pthread.h>
#include <event2/bufferevent.h>
#include <event2/buffer.h>
#include <event2/listener.h>
#include <event2/util.h>
#include <glib.h>
#include <time.h>
#include <assert.h>
#include <libxml/parser.h>


typedef struct get_book {
    gchar   *name;
    gchar   *kind;
    gchar   *price;
    gchar   *time;
}get_book;

enum rule{
    ADD,
    DEL,
    MOD,
    INQ
}rule_index;

gint status = -1;
gint i = 0;
gchar  *addinfo[3];
gchar  *modinfo[3];
gchar  *delinfo;
static gchar *book_kind[] = {"fiction", "life", "science", "sport"};
static gchar mod_info[50][50];
static gint set = 0;
static gchar buf[30];
static gchar xml_buf[1800];
static gint buf_pos = 0;
pthread_mutex_t lock;
pthread_cond_t cond;
GAsyncQueue *queue = NULL;
gint *tmp;
gint tmp_count = 0;

void add_book_info(GList **book_info, get_book *book)
{
    book_info[0] = g_list_append(book_info[0], book->name);
    book_info[1] = g_list_append(book_info[1], book->kind);
    book_info[2] = g_list_append(book_info[2], book->price);
    book_info[3] = g_list_append(book_info[3], book->time);
}
void del_book_info(GList **book_info, get_book *book)
{
    book_info[0] = g_list_remove(book_info[0], book->name);
    book_info[1] = g_list_remove(book_info[1], book->kind);
    book_info[2] = g_list_remove(book_info[2], book->price);
    book_info[3] = g_list_remove(book_info[3], book->time);
}

void func(gpointer data, gpointer user_data)
{
    gchar *str = (gchar *)data;
    snprintf(buf, 30, "%-30s\n", str);
    strncat(xml_buf, buf, 30);
}

void display_book(GList **list, GFunc func, gpointer user_data)
{
    memset(xml_buf, 0, 1024);
    for (i = 0; i < 4; i++) {
        g_list_foreach(list[i], func, NULL);
        strncat(xml_buf, "\n", 1);           
    }
}

void mod_index_info(GList **book_info, gint mod_key, GList *new_total, gchar *mod_name, gchar *mod_later_name)
{
    gint pos = 0, seat = 0, value = 0;
    gchar *a = NULL;
    gchar *mod_book = NULL;
    GList *mod_list = NULL;
    get_book *new_total_book = NULL;
    
    for (mod_list = book_info[mod_key]; mod_list != NULL; mod_list = mod_list->next) {
        a = (gchar *)mod_list->data;
        if (mod_key == 1) {

            if (strcmp(mod_later_name, "1") == 0)
                mod_later_name = (gchar *)book_kind[0];
            else if (strcmp(mod_later_name, "2") == 0)
                mod_later_name = (gchar *)book_kind[1];
            else if (strcmp(mod_later_name, "3") == 0)
                mod_later_name = (gchar *)book_kind[2];
            else if (strcmp(mod_later_name, "4") == 0)
                mod_later_name = (gchar *)book_kind[3];
        }
        if (memcmp(mod_name, a, strlen(mod_name)) == 0) {
	    memcpy(mod_info[set], mod_later_name, strlen(mod_later_name));
            mod_book = (gchar *)mod_list->data;
            seat = value;
            mod_list->data = mod_info[set];
            while(pos < seat-1) {
                if (seat == 1)
                    break;
                new_total = new_total->next;
                pos++;
            }
            new_total_book = (get_book *)new_total->data;
            if (mod_key == 0)
	        new_total_book->name = mod_info[set];
            else if (mod_key == 1)
                new_total_book->kind = mod_info[set];
            else if (mod_key == 2)
                new_total_book->price = mod_info[set];
            set++;
        }
        value++;
    }

}

void *library(void *arg)
{
    gint i = 0;
    gchar *key_array[5] = {"1","2","3","4","5"};
    gchar *book[4] = {"name","kind", "price", "time"};
    GList *book_info[4];
    GList *new_total = NULL;
    struct tm *p;
    time_t timep;
    gchar utc_buf[100];
    get_book new_book[100];
    gint j = 0;
    GList *total_list = NULL;
    GList *index_value = NULL;
    gchar kind_buf[100];
    gint count = 0;

    for (i = 0; i < 100; i++) {
        new_book[i].name = (gchar *)malloc(30);
        new_book[i].kind = (gchar *)malloc(30);
        new_book[i].price = (gchar *)malloc(30);
        new_book[i].time = (gchar *)malloc(30);
    }

    for(i = 0; i < 4; i++) {
        book_info[i] = g_list_append(book_info[i], book[i]);
    }

    GHashTable *table = g_hash_table_new(g_str_hash, g_str_equal);

    for (i = 0; i < 4; i++) {
        g_hash_table_insert(table, key_array[i], book_info[i]->data);
    }
    gint *usr_data;
    while(1) {
        time(&timep);
        p = gmtime(&timep);
    	g_async_queue_lock (queue);
	usr_data = (gint *)g_async_queue_pop_unlocked(queue);
	g_printf("%s pop: %d\n", __func__, *usr_data);
        g_free (usr_data);
        g_printf ("queue length %d\n", g_async_queue_length_unlocked(queue));
        if (status == ADD) {
            pthread_mutex_lock(&lock);
            new_book[j].name  = addinfo[0]; 
            new_book[j].kind  = addinfo[1];
            new_book[j].price = addinfo[2];

            sprintf(utc_buf, "%d:%d:%d", 8+p->tm_hour, p->tm_min, p->tm_sec);
            memcpy(new_book[j].time, utc_buf, strlen(utc_buf)+1);

            if (strcmp(addinfo[1], "1") == 0)
                new_book[j].kind = (gchar *)book_kind[0];
            else if (strcmp(addinfo[1], "2") == 0)
                new_book[j].kind = (gchar *)book_kind[1];
            else if (strcmp(addinfo[1], "3") == 0)
                new_book[j].kind = (gchar *)book_kind[2];
            else if (strcmp(addinfo[1], "4") == 0)
                new_book[j].kind = (gchar *)book_kind[3];
	    add_book_info(book_info, &new_book[j]);

            total_list = g_list_append(total_list, &new_book[j]);
            display_book(book_info, func, NULL);
            ++j;
	    pthread_mutex_unlock(&lock);
	    pthread_cond_signal(&cond);
        } else if (status == DEL) {
	    //pthread_mutex_lock(&lock);
            GList *list = NULL;
            GList *new_list = NULL;
            get_book *del_book = NULL;
            gboolean del_ack = FALSE;
            if (total_list == NULL) {
                printf("totallist NULL\n");
            }

            for (list = total_list; list != NULL; list = list->next) {
                del_book = (get_book *)list->data;
		if (strcmp(del_book->name, delinfo) == 0) {
                    del_ack = TRUE;
                    --count;
		    total_list = g_list_remove(total_list, del_book);
                    del_book_info(book_info, del_book);

                    for (index_value = total_list; index_value != NULL; index_value = index_value->next) {
                        get_book *op = (get_book *)index_value->data;
                    }

		}
            }
            if (del_ack == FALSE) {
		snprintf(xml_buf, 1024, "you input name is not in the table");
               // g_printf("you input name is not in the table\n");
            }
	    display_book(book_info, func, NULL);
	    pthread_mutex_unlock(&lock);
            pthread_cond_signal(&cond);

        } else if (status == MOD) {
	    pthread_mutex_lock(&lock); 
            new_total = total_list;
	    gint mod_key;
	 
	    mod_key = atoi(modinfo[0]);
	    if (mod_key == 1) {
                    mod_index_info(book_info, mod_key-1, new_total, modinfo[1], modinfo[2]);
            } else if (mod_key == 2) {
                if (strcmp(modinfo[1], "1") == 0)
                    modinfo[1] = (gchar *)book_kind[0];
                else if (strcmp(modinfo[1], "2") == 0)
                    modinfo[1] = (gchar *)book_kind[1];
                else if (strcmp(modinfo[1], "3") == 0)
                    modinfo[1] = (gchar *)book_kind[2];
                else if (strcmp(modinfo[1], "4") == 0)
                    modinfo[1] = (gchar *)book_kind[3];

                mod_index_info(book_info, mod_key-1, new_total, modinfo[1], modinfo[2]);
            } else if (mod_key == 3) {
                mod_index_info(book_info, mod_key-1, new_total, modinfo[1], modinfo[2]);
            }
	    display_book(book_info, func, NULL);
	    pthread_mutex_unlock(&lock);
            pthread_cond_signal(&cond);
        } else if (status == INQ) {
            pthread_mutex_lock(&lock);
	    display_book(book_info, func, NULL);
            g_printf("%s\n", xml_buf);
	    pthread_mutex_unlock(&lock);
            pthread_cond_signal(&cond);
        }
        g_async_queue_unlock(queue);
    }
    for (i = 0; i < 100; i++) {
        free(new_book[i].name);
        free(new_book[i].kind);
        free(new_book[i].price);
        free(new_book[i].time);
    }
}


char *xml_get_data(xmlNodePtr curNode)
{
    gint p = 0;    
    xmlNodePtr curdataNode = NULL;
    
    curNode = curNode->xmlChildrenNode;
    while (curNode != NULL) {
        if (!xmlStrcmp(curNode->name, (const xmlChar*)"add")) {
	    p = 0;
            curdataNode = curNode->xmlChildrenNode;
            while (curdataNode != NULL) {
                addinfo[p] = (char *)xmlNodeGetContent(curdataNode);
		curdataNode = curdataNode->next;
                p++;		
            }
            status = ADD;
        } else if (!xmlStrcmp(curNode->name, (const xmlChar*)"del")) {
            curdataNode = curNode->xmlChildrenNode;
	    while (curdataNode != NULL) {
		delinfo = (char *)xmlNodeGetContent(curdataNode);
                curdataNode = curdataNode->next;
            }
            status = DEL;
	} else if (!xmlStrcmp(curNode->name, (const xmlChar*)"mod")) {
	    p = 0;
            curdataNode = curNode->xmlChildrenNode;
            while (curdataNode != NULL) {
                modinfo[p] = (char *)xmlNodeGetContent(curdataNode);
                curdataNode = curdataNode->next;
		p++;
            }
	    status = MOD;
	} else if (!xmlStrcmp(curNode->name, (const xmlChar*)"inq")) {
	    status = INQ;
        }
        curNode = curNode->xmlChildrenNode;
    }
        
}

void read_cb(struct bufferevent *bev, void *arg)
{
    char buf[1024] = {0};
    char *xml_data = NULL; 
    xmlDocPtr doc = NULL;
    xmlNodePtr curNode = NULL;
    char senddata[2048*2];
    char sendxmldata[1024*2];
    char *response = NULL;

    xmlKeepBlanksDefault(0);
  
    bufferevent_read(bev, buf, sizeof(buf));
    //printf("recv buf :%s\n", buf);
    xml_data = strstr(buf, "<");    

    doc = xmlReadMemory(xml_data, strlen(xml_data), NULL, "UTF-8", XML_PARSE_RECOVER);
    if (doc == NULL)
        printf("xml doc is not parsed successful.\n");
    curNode = xmlDocGetRootElement(doc);
    if (curNode == NULL)
        printf("the doc is empty.\n");
    
    xml_get_data(curNode);
    g_async_queue_lock(queue);

    tmp = (gint *)g_new0(gint, 1);
    *tmp = ++tmp_count;
    g_async_queue_push_unlocked (queue, tmp);
    g_async_queue_unlock(queue);
    
    if (status == ADD) 
	response = "addResponse";
    else if (status == DEL) 
	response = "delResponse";
    else if (status == MOD) 
	response = "modResponse";
    else if (status == INQ) 
	response = "inqResponse";
    
    pthread_mutex_lock(&lock);
    pthread_cond_wait(&cond, &lock);
    status = -1;

    snprintf(sendxmldata, 1024*2, "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\r\n\
<SOAP-ENV:Envelope xmlns:SOAP-ENV=\"http://schemas.xmlsoap.org/soap/envelope/\" xmlns:SOAP-ENC=\"http://schemas.xmlsoap.org/soap/encoding/\" \
xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\" xmlns:xsd=\"http://www.w3.org/2001/XMLSchema\" xmlns:ns2=\"urn:calc\">\r\n\
        <SOAP-ENV:Body SOAP-ENV:encodingStyle=\"http://schemas.xmlsoap.org/soap/encoding/\">\r\n\
                <ns2:%s>\r\n\
                        <result>%s</result>\r\n\
                </ns2:%s>\r\n\
        </SOAP-ENV:Body>\r\n\
</SOAP-ENV:Envelope>", response, xml_buf, response);

    snprintf(senddata, 2048*2, "Status: 200 OK\r\n\
Server: gSOAP/2.8\r\n\
X-Frame-Options: SAMEORIGIN\r\n\
Content-Type: text/xml; charset=utf-8\r\n\
Content-Length: %d\r\n\
Connection: close\r\n\r\n\%s", strlen(sendxmldata), sendxmldata);

    bufferevent_write(bev, senddata, strlen(senddata));
    printf("send message client \n");
    printf("%s\n", senddata);
    pthread_mutex_unlock(&lock);
    xmlFreeDoc(doc);
}

void write_cb(struct bufferevent *bev, void *arg)
{
    printf("succes send\n");
}

void event_cb(struct bufferevent *bev, short events, void *arg)
{
    printf("proce\n");
    if (events & BEV_EVENT_EOF)
	printf("connection closed\n");
    else if (events & BEV_EVENT_ERROR)
	printf("some other error\n");
    else if (events & BEV_EVENT_CONNECTED)
        printf("new client connect\n");
}

void read_terminal(evutil_socket_t fd, short what, void *arg)
{
    
    char buf[1024] = {0};
    int len = read(fd, buf, sizeof(buf));
                
    struct bufferevent* bev = (struct bufferevent*)arg;                   
    
    bufferevent_write(bev, buf, len+1);
}

void listen_cb(struct evconnlistener *listener,	evutil_socket_t fd,
		struct sockaddr *addr, int len, void *ptr)
{
    
    struct event_base *base = evconnlistener_get_base(listener);
    struct bufferevent *bev = NULL;
    struct event* ev = NULL;
    
    bev = bufferevent_socket_new(base, fd, BEV_OPT_CLOSE_ON_FREE);
    
    bufferevent_setcb(bev, read_cb, write_cb, event_cb, NULL);
    bufferevent_enable(bev, EV_READ|EV_WRITE);
    ev = event_new(base, STDIN_FILENO, EV_READ | EV_PERSIST, read_terminal, bev);

    event_add(ev, NULL);
}


int main(void)
{
    struct event_base *base = event_base_new();
    pthread_t t;
    pthread_mutex_init(&lock,NULL);
    queue = g_async_queue_new ();

    struct sockaddr_in serv;
    memset(&serv, 0, sizeof(serv));
    serv.sin_family = AF_INET;
    serv.sin_port = htons(50000);
    serv.sin_addr.s_addr = htonl(INADDR_ANY);

    struct evconnlistener *listen = NULL;
    listen = evconnlistener_new_bind(base, listen_cb, NULL, 
					LEV_OPT_CLOSE_ON_FREE | LEV_OPT_REUSEABLE,
					-1, (struct sockaddr*)&serv, sizeof(serv));
    pthread_create(&t, NULL, library, NULL);

    event_base_dispatch(base);
   
    pthread_mutex_destroy(&lock);
    pthread_cond_destroy(&cond); 
    evconnlistener_free(listen);
    event_base_free(base);
    return 0;
}
