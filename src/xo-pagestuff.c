#include <gcrypt.h>
#include <stdio.h>
#include <stdlib.h>

#include <gtk/gtk.h>
#include <sqlite3.h>

// need to add these libraries to the makefile
// -lgcrypt -lgpg-error -lsqlite3

char SHA1_BG[41] = "NOTHING";

// ref: http://www.sqlite.org/quickstart.html
static int get_last_open_page_callback(int *page_number, int argc, char **argv, char **azColName){
  int i;
  for(i=0; i<argc; i++){
      if(strcmp(azColName[i], "page_number") == 0) {
          *page_number = atoi(argv[i]);
      }
  }
  return 0;
}
int sql3_query_last_open_page(int pageno) {
    // look for xournal database
    gchar *xournaldb;
    xournaldb = g_build_filename(g_get_home_dir(), CONFIG_DIR, "xournal.db", NULL);
    sqlite3 *db;
    char *zErrMsg = 0;
    int rc;
    int rtn = 0;

    rc = sqlite3_open(xournaldb, &db);
    if( rc ){
      fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
      sqlite3_close(db);
      exit(1);
    }
    rc = sqlite3_exec(db, "CREATE TABLE IF NOT EXISTS last_open_page(sha1 CHAR(40), page_number INTEGER, PRIMARY KEY(sha1))", NULL, 0, &zErrMsg);
    if( rc!=SQLITE_OK ){
      fprintf(stderr, "SQL error: %s\n", zErrMsg);
      sqlite3_free(zErrMsg);
    }

    char querybuf[120];
    if(pageno == 0) {
        sprintf(querybuf, "SELECT page_number FROM last_open_page WHERE sha1 = '%s'", SHA1_BG);
        rc = sqlite3_exec(db, querybuf, get_last_open_page_callback, &rtn, &zErrMsg);
    } else {
        sprintf(querybuf, "INSERT OR REPLACE INTO last_open_page (sha1, page_number) VALUES ('%s', %06d)", SHA1_BG, pageno);
        rc = sqlite3_exec(db, querybuf, NULL, 0, &zErrMsg);
    }

    if( rc!=SQLITE_OK ){
      fprintf(stderr, "SQL error: %s\n", zErrMsg);
      sqlite3_free(zErrMsg);
    }
    sqlite3_close(db);
    return rtn;
}

int get_last_open_page() {
    return sql3_query_last_open_page(0);
}
int set_last_open_page(int pg) {
    return sql3_query_last_open_page(pg);
}


void init_file_hash(){
    
    gchar *tmppath, *tmpfn;
    
    tmpfn = g_build_filename(bgpdf.filename->s, NULL);
    
    FILE * fp;
    long filesize;
    char * filebuf;
    if(fp = fopen(bgpdf.filename->s, "rb")) {
        fseek(fp, 0, SEEK_END);
        filesize = ftell(fp);
        fseek(fp, 0, SEEK_SET);
        filebuf = (char*) malloc(filesize);
        if(filebuf == NULL) {
            fprintf( stderr, "failed to malloc!!!\n");
            exit(1);
        }
        fread(filebuf, filesize, 1, fp);
        fclose(fp);
    
        // ref http://ubuntuforums.org/showpost.php?p=2014108&postcount=9
        /* Length of resulting sha1 hash - gcry_md_get_algo_dlen
        * returns digest length for an algo */
        int hash_len = gcry_md_get_algo_dlen( GCRY_MD_SHA1 );
        
        /* output sha1 hash - this will be binary data */
        unsigned char hashbuf[ hash_len ]; 
        /* output sha1 hash - converted to hex representation
        * 2 hex digits for every byte + 1 for trailing \0 */
        char *out = (char *) malloc( sizeof(char) * ((hash_len*2)+1) );
        char *p = SHA1_BG;
        /* calculate the SHA1 digest. This is a bit of a shortcut function
        * most gcrypt operations require the creation of a handle, etc. */
        gcry_md_hash_buffer( GCRY_MD_SHA1, hashbuf, filebuf, filesize );
        free(filebuf);
        
        /* Convert each byte to its 2 digit ascii
        * hex representation and place in out */
        int i;
        for ( i = 0; i < hash_len; i++, p += 2 ) {
            snprintf ( p, 3, "%02x", hashbuf[i] );
        }
    }
    // printf("filehash is %s\n", SHA1_BG);
}

