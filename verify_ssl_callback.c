#include "http-fetch.h"

/* --- */

int verify_ssl_callback(int ok, X509_STORE_CTX *context)

{
    int rc = ok, namelen, done, pos, x509_err, x509_depth, exn, nid_num, excount, altcount, alt;
    unsigned long hold_err;
    unsigned char *nameval = 0;
    const char *x509_emsg = 0;
    struct plan_data *plan = 0;
    struct output_options *out = 0;
    struct display_settings *disp = 0;
    struct fetch_status *status = 0;
    X509 *cert;
    X509_NAME *subject = 0;
    X509_NAME_ENTRY *common = 0;
    X509_EXTENSION *cex = 0;
    ASN1_STRING *cdata = 0;
    ASN1_IA5STRING *altdns = 0;
    GENERAL_NAME *altval = 0;
    STACK_OF(GENERAL_NAME) *altnames = 0;

    /* We need the plan data to find info on this run */
    plan = register_current_plan( 0);
    out = plan->out;
    disp = plan->disp;
    status = plan->status;

    if( !ok) 
    {
        /* Get the cert, the error code and the depth at which the error was detected */
        x509_err = X509_STORE_CTX_get_error( context);
        x509_depth = X509_STORE_CTX_get_error_depth( context);

        if( x509_err == X509_V_OK) rc = 1;
        else if( plan->target->insecure_cert) rc = 1;
        else
        {
            x509_emsg = X509_verify_cert_error_string( x509_err);
            if( x509_emsg) if( !*x509_emsg) x509_emsg = 0;
            hold_err = ERR_peek_error();
            if( hold_err || x509_emsg)
            {
                status->end_errno = hold_err;
                if( x509_emsg) status->err_msg = strdup( x509_emsg);
                else (void) stash_ssl_err_info( status, hold_err);
	    }
            else
            {
                status->end_errno = errno;
                status->err_msg = strdup( EMSG_SSL_HANDSHAKE_FAIL);
	    }
	}

        if( out->debug_level >= DEBUG_MEDIUM1)
        {
            x509_emsg = X509_verify_cert_error_string( x509_err);
            fprintf( out->info_out, "%sSSL verification, err=%d, depth=%d msg'%s'\n",
              disp->line_pref, x509_err, x509_depth, x509_emsg);
	}
    }

    if( out->debug_level >= DEBUG_HIGH1)
    {
        cert = X509_STORE_CTX_get_current_cert( context);
        if( cert)
        {
            subject = X509_get_subject_name( cert);
            done = 0;
            pos = -1;
            for( ; !done; )
            {
                pos = X509_NAME_get_index_by_NID( subject, NID_commonName, pos);
                if( pos == -1) done = 1;
                else
                {
                    common = X509_NAME_get_entry( subject, pos);
                    if( common)
                    {
                        cdata = X509_NAME_ENTRY_get_data( common);
                        if( cdata)
                        {
                            namelen = ASN1_STRING_length( cdata);
                            nameval = ASN1_STRING_data( cdata);
                            fprintf( out->info_out, "%sSSL cert found, common name '",
                              disp->line_pref);
                            fwrite( nameval, 1, namelen, out->info_out);
                            fprintf( out->info_out, "'\n");
                        }
                    }
                }
            }

            excount = X509_get_ext_count( cert);
            for( exn = 0; exn < excount; exn++)
            {
                cex = X509_get_ext( cert, exn);
                nid_num = OBJ_obj2nid(X509_EXTENSION_get_object(cex));
                fprintf( out->info_out, "%sSSL ext, nid=%d name'%s'\n", disp->line_pref, nid_num, OBJ_nid2ln( nid_num));
                fprintf( out->info_out, "%sSSL ext, val'", disp->line_pref);
                X509V3_EXT_print_fp( out->info_out, cex, 0, 1);
                fprintf( out->info_out, "'\n");
            }

            pos = -1;
            altnames = X509_get_ext_d2i( cert, NID_subject_alt_name, 0, &pos);

            for( ; altnames; )
            {
                altcount = sk_GENERAL_NAME_num( altnames);
                for( alt = 0; alt < altcount; alt++)
                {
                    altval = sk_GENERAL_NAME_value( altnames, alt);
#ifdef __APPLE__
                    altdns = altval->d.ia5;
#else
                    altdns = (ASN1_IA5STRING *) GENERAL_NAME_get0_value( altval, 0);
#endif
                    if( altval->type == GEN_DNS) fprintf( out->info_out, "%sSSL ext, alt-name %d. '%s'\n", disp->line_pref, alt, altdns->data);
                }
                GENERAL_NAMES_free( altnames);
                altnames = X509_get_ext_d2i( cert, NID_subject_alt_name, 0, &pos);
            }

            if( altnames) GENERAL_NAMES_free( altnames);
        }
    }

    return( rc);
}
