/*
 * Unix PuTTY main program.
 */

#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <assert.h>
#include <unistd.h>

#include "putty.h"
#include "storage.h"

/*
 * TODO:
 * 
 *  - Go through all the config options and ensure they can all be
 *    configured and reconfigured properly.
 *     + icon title appears to be a non-option on Unix.
 *     + Why the hell did I faff about disabling two of the vtmode
 * 	 options? The rest aren't used either in pterm! Work out
 * 	 whether they should be, and how they can be.
 *     + Refresh in the codepage combo is badly broken.
 *     + `Don't translate line drawing chars' ?? What is this crap?
 * 	 It does nothing at all, and where's the option to paste as
 * 	 lqqqk? What was I smoking?
 *
 *  - Better control of the individual config box features.
 *     + SSH packet logging shouldn't be mentioned in pterm, and in
 * 	 fact not PuTTYtel either.
 *     + Keepalives, and the Connection panel in general, shouldn't
 * 	 crop up in pterm. (And perhaps also not mid-session in
 * 	 rlogin and raw?)
 */

/*
 * Clean up and exit.
 */
void cleanup_exit(int code)
{
    /*
     * Clean up.
     */
    sk_cleanup();
    random_save_seed();
    exit(code);
}

Backend *select_backend(Config *cfg)
{
    int i;
    Backend *back = NULL;
    for (i = 0; backends[i].backend != NULL; i++)
	if (backends[i].protocol == cfg->protocol) {
	    back = backends[i].backend;
	    break;
	}
    assert(back != NULL);
    return back;
}

int cfgbox(Config *cfg)
{
    return do_config_box("PuTTY Configuration", cfg, 0);
}

static int got_host = 0;

const int use_event_log = 1, new_session = 1, saved_sessions = 1;

int process_nonoption_arg(char *arg, Config *cfg)
{
    char *p, *q = arg;

    if (got_host) {
        /*
         * If we already have a host name, treat this argument as a
         * port number. NB we have to treat this as a saved -P
         * argument, so that it will be deferred until it's a good
         * moment to run it.
         */
        int ret = cmdline_process_param("-P", arg, 1, cfg);
        assert(ret == 2);
    } else if (!strncmp(q, "telnet:", 7)) {
        /*
         * If the hostname starts with "telnet:",
         * set the protocol to Telnet and process
         * the string as a Telnet URL.
         */
        char c;

        q += 7;
        if (q[0] == '/' && q[1] == '/')
            q += 2;
        cfg->protocol = PROT_TELNET;
        p = q;
        while (*p && *p != ':' && *p != '/')
            p++;
        c = *p;
        if (*p)
            *p++ = '\0';
        if (c == ':')
            cfg->port = atoi(p);
        else
            cfg->port = -1;
        strncpy(cfg->host, q, sizeof(cfg->host) - 1);
        cfg->host[sizeof(cfg->host) - 1] = '\0';
        got_host = 1;
    } else {
        /*
         * Otherwise, treat this argument as a host name.
         */
        p = arg;
        while (*p && !isspace((unsigned char)*p))
            p++;
        if (*p)
            *p++ = '\0';
        strncpy(cfg->host, q, sizeof(cfg->host) - 1);
        cfg->host[sizeof(cfg->host) - 1] = '\0';
        got_host = 1;
    }
    return 1;
}

char *make_default_wintitle(char *hostname)
{
    return dupcat(hostname, " - PuTTY", NULL);
}

int main(int argc, char **argv)
{
    extern int pt_main(int argc, char **argv);
    sk_init();
    flags = FLAG_VERBOSE | FLAG_INTERACTIVE;
    default_protocol = be_default_protocol;
    /* Find the appropriate default port. */
    {
	int i;
	default_port = 0; /* illegal */
	for (i = 0; backends[i].backend != NULL; i++)
	    if (backends[i].protocol == default_protocol) {
		default_port = backends[i].backend->default_port;
		break;
	    }
    }
    return pt_main(argc, argv);
}