#include <stdio.h>
#include <glib.h>
#include <locale.h>
#include <string.h>

#include "himd.h"

char * codecstr(struct trackinfo * track)
{
    static char buffer[5];
    if(track->codec_id == CODEC_LPCM)
        return "LPCM";
    if(track->codec_id == CODEC_LOSSY && track->codecinfo[0] == 3)
        return "MPEG";
    sprintf(buffer,"%4d",track->codec_id);
    return buffer;
}

char * get_locale_str(struct himd * himd, int idx)
{
    char * str, * outstr;
    str = himd_get_string_utf8(himd, idx, NULL);
    if(!str)
        return NULL;

    outstr = g_locale_from_utf8(str,-1,NULL,NULL,NULL);
    himd_free(str);
    return outstr;
}

void himd_trackdump(struct himd * himd)
{
    int i;
    for(i = 1;i < 2048;i++)
    {
        struct trackinfo * const t = &himd->tracks[i];
        if(t->firstpart != 0)
        {
            char * title = get_locale_str(himd, t->title);
            char * artist = get_locale_str(himd, t->artist);
            char * album = get_locale_str(himd, t->album);
            printf("%4d: %d:%d %s %s:%s (%s %d)\n",
                    i, t->seconds/60,t->seconds % 60, codecstr(t),
                    artist, title, album, t->trackinalbum);
            g_free(title);
            g_free(artist);
            g_free(album);
        }
    }
}

void himd_stringdump(struct himd * himd)
{
    int i;
    for(i = 1;i < 4096;i++)
    {
        char * str;
        int type;
        if((str = himd_get_string_utf8(himd, i, &type)) != NULL)
        {
            char * typestr;
            char * outstr;
            switch(type)
            {
                case STRING_TYPE_TITLE: typestr="Title"; break;
                case STRING_TYPE_ARTIST: typestr="Artist"; break;
                case STRING_TYPE_ALBUM: typestr="Album"; break;
                case STRING_TYPE_GROUP: typestr="Group"; break;
                default: typestr=""; break;
            }
            outstr = g_locale_from_utf8(str,-1,NULL,NULL,NULL);
            printf("%4d: %-6s %s\n", i, typestr, outstr);
            g_free(outstr);
            himd_free(str);
        }
        else if(himd->status != HIMD_ERROR_NOT_STRING_HEAD)
            printf("%04d: ERROR %s\n", i, himd->statusmsg);
    }
}

void himd_dumpdiscid(struct himd * h)
{
    int i;
    printf("Disc ID: ");
    for(i = 0;i < 16;++i)
        printf("%02X",h->discid[i]);
    puts("");        
}

int main(int argc, char ** argv)
{
    struct himd * h;
    setlocale(LC_ALL,"");
    if(argc < 2)
    {
        fputs("Please specify mountpoint of image\n",stderr);
        return 1;
    }
    h = himd_new(argv[1]);
    if(h->status != HIMD_OK)
    {
        puts(h->statusmsg);
        return 1;
    }
    if(argc == 2 || strcmp(argv[2],"strings") == 0)
        himd_stringdump(h);
    else if(strcmp(argv[2],"tracks") == 0)
        himd_trackdump(h);
    else if(strcmp(argv[2],"discid") == 0)
        himd_dumpdiscid(h);
    return 0;
}