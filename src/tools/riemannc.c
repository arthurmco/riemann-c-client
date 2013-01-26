#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <riemann/common.h>
#include <riemann/event.h>
#include <riemann/message.h>
#include <riemann/client.h>
#include <riemann/attribute.h>

#include <getopt.h>

enum {
        OPT_SERVER,
        OPT_PORT,
        OPT_TCP,
        OPT_UDP,
        OPT_TIME,
        OPT_STATE,
        OPT_SERVICE,
        OPT_HOST,
        OPT_DESCRIPTION,
        OPT_TAGS,
        OPT_TTL,
        OPT_ATTRIBUTES,
        OPT_METRIC_SINT64,
        OPT_METRIC_F,
        OPT_METRIC_D,
        OPT_FORMAT,
        OPT_QUERY_STRING,
};
        
static struct option send_opts[] = {
        {"server", required_argument, 0, OPT_SERVER},
        {"port", required_argument, 0, OPT_PORT},
        {"tcp", required_argument, 0, OPT_TCP},
        {"udp", required_argument, 0, OPT_UDP},
        {"time", required_argument, 0, OPT_TIME},
        {"state", required_argument, 0, OPT_STATE},
        {"service", required_argument, 0, OPT_SERVICE},
        {"host", required_argument, 0, OPT_HOST},
        {"description", required_argument, 0, OPT_DESCRIPTION},
        {"tags", required_argument, 0, OPT_TAGS},
        {"ttl", required_argument, 0, OPT_TTL},
        {"attributes", required_argument, 0, OPT_ATTRIBUTES},
        {"metric_sint64", required_argument, 0, OPT_METRIC_SINT64},
        {"metric_f", required_argument, 0, OPT_METRIC_F},
        {"metric_d", required_argument, 0, OPT_METRIC_D},
        {0, 0, 0, 0},
};

static struct option query_opts[] = {
        {"server", required_argument, 0, OPT_SERVER},
        {"port", required_argument, 0, OPT_PORT},
        {"tcp", required_argument, 0, OPT_TCP},
        {"string", required_argument, 0, OPT_QUERY_STRING},
        {"format", required_argument, 0, OPT_FORMAT},
        {0, 0, 0, 0},
};

/* Default options */
static char *server = "localhost";
static int port = 5555;
static int cli_type = TCP;
static char *format = "%h/%s=%mf (%S)";
static char *query = "true";

int riemannc_send(int argc, char **argv);
int riemannc_query(int argc, char **argv);
void riemannc_usage(void);

int main(int argc, char **argv)
{
        if (argc <= 1) {
                riemannc_usage();
                exit(EXIT_FAILURE);
        }
        
        if (!strncmp("send", argv[1], 4)) {
                argc--;
                argv++;
                return riemannc_send(argc, argv);
        } else if (!strncmp("query", argv[1], 5)) {
                argc--;
                argv++;
                return riemannc_query(argc, argv);
        }

        return 0;
}

void riemannc_usage(void)
{
        puts("usage");          /*  do this  */
}

int riemannc_send(int argc, char **argv)
{
        int ch;
        int optind;
        riemann_message_t msg = RIEMANN_MSG_INIT;
        riemann_message_t *resp = NULL;
        riemann_client_t cli;
        riemann_event_t **evts;
        int i;
        char **tags;
        int n_tags;
        riemann_attribute_pairs_t *pairs = NULL;
        int n_attrs;
        int error;

        evts = riemann_event_alloc_events(1);
        assert(evts);
        evts[0] = riemann_event_alloc_event();
        assert(evts[0]);
        riemann_event_init(evts[0]);
        while ((ch = getopt_long_only(argc, argv, 
                                      "", 
                                      send_opts, &optind)) != -1) {
                switch (ch) {
                case OPT_SERVER:
                        server = optarg;
                        break;
                case OPT_PORT:
                        port = atoi(optarg);
                        break;
                case OPT_TCP:
                        cli_type = TCP;
                        break;
                case OPT_UDP:
                        cli_type = UDP;
                        break;
                case OPT_TIME:
                        riemann_event_set_time(evts[0], (int64_t) atoll(optarg));
                        break;
                case OPT_STATE:
                        riemann_event_set_state(evts[0], optarg);
                        break;
                case OPT_SERVICE:
                        riemann_event_set_service(evts[0], optarg);
                        break;
                case OPT_HOST:
                        riemann_event_set_host(evts[0], optarg);
                        break;
                case OPT_DESCRIPTION:
                        riemann_event_set_description(evts[0], optarg);
                        break;
                case OPT_TAGS:
                        tags = strtoknize(optarg, strlen(optarg), ",", 1, &n_tags);
                        assert(tags);
                        riemann_event_set_tags(evts[0], (const char **)tags, n_tags);
                        break;
                case OPT_TTL:
                        riemann_event_set_ttl(evts[0], strtof(optarg, NULL));
                        break;
                case OPT_ATTRIBUTES:
                        do {            
                                char **attrs = NULL;
                                char **pair = NULL;
                                int i;

                                attrs = strtoknize(optarg, strlen(optarg) + 1, ",", 1, &n_attrs);
                                pairs = malloc(sizeof (riemann_attribute_pairs_t) * n_attrs);
                                /**
                                 * OMG, this is so ugly
                                 */
                                for (i = 0; i < n_attrs; i++) {
                                        pair = strtoknize(attrs[i], strlen(attrs[i]) + 1, "=", 1, NULL);
                                        assert(pair);
                                        pairs[i].key = strdup(pair[0]);
                                        pairs[i].value = strdup(pair[1]);
                                        free(pair[0]);
                                        free(pair[1]);
                                        free(pair);
                                }
                        } while (0);
                        riemann_event_set_attributes(evts[0], pairs, n_attrs);
                        break;
                case OPT_METRIC_SINT64:
                        riemann_event_set_metric_sint64(evts[0], atoll(optarg));
                        break;
                case OPT_METRIC_F:
                        riemann_event_set_metric_f(evts[0], strtof(optarg, NULL));
                        break;
                case OPT_METRIC_D:
                        riemann_event_set_metric_d(evts[0], strtod(optarg, NULL));
                        break;
                case '?':
                        fprintf(stderr, "Unknown option found\n");
                        riemannc_usage();
                        exit(EXIT_FAILURE);
                default:
                        break;
                }
        }
        
        riemann_message_set_events(&msg, evts, 1);

        riemann_client_init(&cli);
        error = riemann_client_connect(&cli, cli_type, server, port);
        if (error) 
                pexit("riemann_client_connect");

        error = riemann_client_send_message(&cli, &msg, 0, NULL);
        if (error) 
                pexit("riemann_client_send_message");
        
        resp = riemann_client_recv_message(&cli, 0, NULL);
        if (!resp)
                pexit("riemann_client_recv_message");
        
        if (!resp->ok) {
                fprintf(stderr, "riemann message error: %s\n", resp->error);
                exit(EXIT_FAILURE);
        }

        puts("sent");
                
        return 0;
}

int riemannc_query(int argc, char **argv)
{
        riemann_client_t cli;
        riemann_message_t msg = RIEMANN_MSG_INIT;
        riemann_message_t *resp = NULL;
        riemann_query_t qry = RIEMANN_QUERY_INIT;
        int i;
        int error;
        char ch;
#ifndef BUFSIZ
# define BUFSIZ 4096
#endif
        char buffer[BUFSIZ];
        
        while ((ch = getopt_long_only(argc, argv, 
                                      "", 
                                      query_opts, &optind)) != -1) {
                switch (ch) {
                case OPT_SERVER:
                        server = optarg;
                        break;
                case OPT_PORT:
                        port = atoi(optarg);
                        break;
                case OPT_QUERY_STRING:
                        query = strdup(optarg);
                        break;
                case OPT_FORMAT:
                        format = strdup(optarg);
                        break;
                case '?':
                        fprintf(stderr, "Unknown option found\n");
                        riemannc_usage();
                        exit(EXIT_FAILURE);
                default:
                        break;
                }
        }

        riemann_query_set_string(&qry, query);
        riemann_message_set_query(&msg, &qry);
        riemann_client_init(&cli);

        error = riemann_client_connect(&cli, TCP, server, port);
        if (error)
                pexit("riemann_client_connect");

        error = riemann_client_send_message(&cli, &msg, 0, NULL);
        if (error)
                pexit("riemann_client_send_message");

        resp = riemann_client_recv_message(&cli, 0, NULL);
        if (!resp)
                pexit("riemann_client_recv_message");

        if (!resp->ok) {
                fprintf(stderr, "Message error %s\n", resp->error);
                exit(EXIT_FAILURE);
        } 
         
        for (i = 0; i < resp->n_events; i++) {
                error = riemann_event_strfevent(buffer, BUFSIZ, format, resp->events[i]);
                if (error) {
                        fprintf(stderr, "riemann_event_strfevent() error\n");
                        exit(EXIT_FAILURE);
                }
                puts(buffer);
        }

        riemann_message_free(resp); /* responses should be freed */
        riemann_client_free(&cli);        
        return 0;
}
 
