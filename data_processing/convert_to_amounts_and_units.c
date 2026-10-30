/* Build: gcc convert_to_amounts_and_units.c -o convert_to_amounts_and_units -lm
   Requires cJSON.c, cJSON.h and recipes.json in the same directory. */
#include <ctype.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "cJSON.h"
#define INPUT "recipes.json"
#define OUTPUT "recipes_simplified.json"
#define N 4096

typedef struct {const char *unit,*names; double scale;} Unit;
static const Unit units[]={
    {"g","kilograms|kilogram|kgs|kg",1000},{"g","milligrams|milligram|mg",.001},
    {"g","grams|gram|gr|g",1},{"ml","millilitres|millilitre|milliliters|milliliter|ml",1},
    {"ml","litres|litre|liters|liter|l",1000},{"ml","decilitres|decilitre|deciliters|deciliter|dl",100},
    {"ml","centilitres|centilitre|centiliters|centiliter|cl",10},
    {"tablespoons","tablespoons|tablespoon|tblsp|tbls|tbsp|tbs",1},
    {"teaspoons","teaspoons|teaspoon|tsp",1},
    {"ml","cups|cup",240},{"ml","fluid ounces|fluid ounce|fl. oz.|fl. oz|fl oz|floz",29.5735295625},
    {"g","ounces|ounce|oz",28.349523125},{"g","pounds|pound|lbs|lb",453.59237},
    {"ml","pints|pint|pt",473.176473},{"ml","quarts|quart|qt",946.352946},
    {"cm","centimetres|centimetre|centimeters|centimeter|cm",1},
    {"cm","inches|inch|\"|″",2.54}
};
static const char *packages="packages|package|packets|packet|bottles|bottle|cartons|carton|sachets|sachet|packs|pack|boxes|box|tins|tin|cans|can|jars|jar|tubs|tub|bags|bag";
static const char *counts="eggs|egg|cloves|clove|pieces|piece|slices|slice|stalks|stalk|sprigs|sprig|heads|head|bunches|bunch|pinches|pinch|handfuls|handful";
typedef struct {double amount; int has_amount; char unit[48],note[N];} Parsed;
static FILE *out;
static void die(const char *s){fprintf(stderr,"Error: %s\n",s);if(out){fclose(out);remove(OUTPUT ".tmp");}exit(1);}
static void *alloc(size_t n){void *p=malloc(n);if(!p)die("Out of memory");return p;}
static const char *ws(const char *p){while(isspace((unsigned char)*p))p++;return p;}
static size_t match(const char *p,const char *list){
    while(*list){size_t n=strcspn(list,"|"),i=0;
        while(i<n && p[i] && tolower((unsigned char)p[i])==tolower((unsigned char)list[i]))i++;
        if(i==n && !isalpha((unsigned char)p[n]))return n;
        list+=n;if(*list)list++;
    }return 0;
}
static int fraction(const char **pp,double *x){
    const char *symbols[]={"½","¼","¾","⅓","⅔","⅛","⅜","⅝","⅞","⅕","⅖","⅗","⅘","⅙","⅚"};
    const double values[]={.5,.25,.75,1./3,2./3,.125,.375,.625,.875,.2,.4,.6,.8,1./6,5./6};
    for(size_t i=0;i<sizeof(values)/sizeof(*values);i++)
        if(!strncmp(*pp,symbols[i],strlen(symbols[i]))){*pp+=strlen(symbols[i]);*x=values[i];return 1;}
    char *e,*last;double a=strtod(*pp,&e);
    if(e==*pp || *e!='/')return 0;
    double b=strtod(e+1,&last);
    if(last==e+1 || b<=0 || !isfinite(a/b) || a<0)return 0;
    *x=a/b;*pp=last;return 1;
}
static int one_number(const char **pp,double *x){
    const char *p=ws(*pp),*words[]={"one and a half","a couple of","half","one","two","an","a"};
    const double values[]={1.5,2,.5,1,2,1,1};
    for(size_t i=0;i<sizeof(values)/sizeof(*values);i++){
        size_t n=match(p,words[i]);if(n){*pp=p+n;*x=values[i];return 1;}
    }
    const char *q=p;if(fraction(&q,x)){*pp=q;return 1;}
    if(!isdigit((unsigned char)*p) && *p!='.')return 0;
    char *end;double v=strtod(p,&end);
    if(end==p || !isfinite(v) || v<0 || *end=='/')return 0;
    q=ws(end);if(*q=='-' && isspace((unsigned char)q[1]))q=ws(q+1);
    double f;if(v>=1 && v < 9000000000000000.0 && (double)(long long)v==v && fraction(&q,&f) && f<1){v+=f;end=(char*)q;}
    *x=v;*pp=end;return 1;
}
static int number(const char **pp,double *x,int *range){
    const char *p=*pp;if(!one_number(&p,x))return 0;
    const char *q=ws(p);size_t n=match(q,"to|through|–|—");
    if(!n && *q=='-' && q[1])n=1;
    if(n){q=ws(q+n);double y;if(one_number(&q,&y)){*x=(*x+y)/2;*range=1;p=q;}}
    *pp=p;return 1;
}
static const Unit *unit(const char **pp){
    const char *p=ws(*pp);
    for(size_t i=0;i<sizeof(units)/sizeof(*units);i++){
        size_t n=match(p,units[i].names);if(n){*pp=p+n;return units+i;}
    }return NULL;
}
static void trim(char *dst,size_t cap,const char *a,const char *b){
    while(a<b && (isspace((unsigned char)*a)||*a==','||*a==';'))a++;
    while(b>a && (isspace((unsigned char)b[-1])||b[-1]==','||b[-1]==';'))b--;
    size_t n=(size_t)(b-a);if(n>=cap)n=cap-1;memcpy(dst,a,n);dst[n]=0;
}
static void note(Parsed *r,const char *a,const char *b){
    char tmp[N];trim(tmp,sizeof(tmp),a,b);if(!*tmp)return;
    size_t n=strlen(r->note);if(n){if(n+2>=N)return;memcpy(r->note+n,", ",3);n+=2;}
    snprintf(r->note+n,N-n,"%s",tmp);
}
static double round_amount(double x){return x>5 && x<9000000000000000.0 ? (double)(long long)(x+0.5) : x;}
static void metric(Parsed *r,double n,const Unit *u){
    if(!isfinite(n*u->scale))return;
    r->amount=round_amount(n*u->scale);r->has_amount=1;snprintf(r->unit,sizeof(r->unit),"%s",u->unit);
}
static int in_parens(const char *p,double *v,const Unit **u,const char **end){
    p=ws(p);if(*p!='(')return 0;
    const char *q=ws(p+1);int range=0;
    if(!number(&q,v,&range) || !(*u=unit(&q)) || !strcmp((*u)->unit,"cm"))return 0;
    q=ws(q);if(*q!=')')return 0;*end=q+1;return 1;
}
static Parsed parse(const char *raw){
    Parsed r={0};if(!raw || !*ws(raw))return r;
    const char *p=ws(raw),*end=raw+strlen(raw),*q=p;double count=1,v;int range=0;
    if(strlen(raw)>=N)return r;
    if(!number(&q,&count,&range)){
        const Unit *u;const char *e;
        if(in_parens(p,&v,&u,&e)){metric(&r,v,u);note(&r,e,end);}
        else {size_t n=match(p,"pinch|dash|handful|sprig");
            if(n){trim(r.unit,sizeof(r.unit),p,p+n);note(&r,p+n,end);}else note(&r,p,end);}
        return r;
    }
    p=ws(q);
    /* Packages with stated weights: 2 x 400g, 2 (400g) tins, 3 400g cans. */
    const char *t=p;int multiplied=0;
    if(*t=='x'||*t=='X'||!strncmp(t,"×",2)){t=ws(t+(*t=='x'||*t=='X'?1:2));multiplied=1;}
    const Unit *u=NULL;const char *e=NULL;
    if(in_parens(t,&v,&u,&e)){
        metric(&r,count*v,u);
        if(multiplied || range)note(&r,raw,end);
        else {char label[N];snprintf(label,sizeof(label),"%g %.*s",count,(int)(end-ws(e)),ws(e));note(&r,label,label+strlen(label));}
        return r;
    }
    {const char *s=t;int inner=0;
        if(number(&s,&v,&inner) && (u=unit(&s)) && strcmp(u->unit,"cm")){
            metric(&r,count*v,u);note(&r,raw,end);return r;
        }
    }
    /* Direct unit, e.g. 1.5 kg, 1 cup, 2 tbsp. */
    t=p;if((u=unit(&t))){metric(&r,count,u);
        if(range)note(&r,raw,end);else note(&r,t,end);
        return r;
    }
    /* 1 can (400g), 2 tins of 400g. */
    size_t n=match(p,packages);
    if(n){t=ws(p+n);size_t of=match(t,"of");if(of)t=ws(t+of);
        if(in_parens(t,&v,&u,&e)){
            metric(&r,count*v,u);note(&r,raw,end);return r;
        }
        const char *s=t;int inner=0;
        if(number(&s,&v,&inner) && (u=unit(&s)) && strcmp(u->unit,"cm")){
            metric(&r,count*v,u);note(&r,raw,end);return r;
        }
    }
    r.amount=round_amount(count);r.has_amount=1;
    n=n?n:match(p,counts);
    if(n){trim(r.unit,sizeof(r.unit),p,p+n);note(&r,p+n,end);}
    else note(&r,p,end);
    if(range && !*r.note)note(&r,raw,end);
    return r;
}
/* Convert weight, volume and length in instructions; preserve spoon measures. */
static char *instructions(const char *src){
    size_t cap=strlen(src)*40+128;char *dst=alloc(cap),*w=dst;
    for(const char *p=src;*p;){
        int boundary=p==src || (!isalnum((unsigned char)p[-1]) && p[-1]!='.');
        if(p>src+1 && p[-1]=='/' && isdigit((unsigned char)p[-2]))boundary=0;
        if(boundary && !isspace((unsigned char)*p)){
            const char *q=p;double v;int range=0;
            if(number(&q,&v,&range)){
                q=ws(q);if(*q=='-' && isalpha((unsigned char)q[1]))q++;
                const Unit *u=unit(&q);
                if(u && strcmp(u->unit,"tablespoons") && strcmp(u->unit,"teaspoons") && isfinite(v*u->scale)){
                    w+=sprintf(w,"%.6g %s",round_amount(v*u->scale),u->unit);p=q;continue;
                }
            }
        }
        *w++=*p++;
    }*w=0;return dst;
}
static const char *get(const cJSON *o,const char *k){
    cJSON *v=cJSON_GetObjectItemCaseSensitive(o,k);return cJSON_IsString(v)?v->valuestring:NULL;
}
static void put(cJSON *o,const char *k,const char *s){if(s && *s && !cJSON_AddStringToObject(o,k,s))die("JSON text");}
static void move(cJSON *dst,const cJSON *src,const char *a,const char *b){put(dst,b,get(src,a));}
static int excluded(const char *id){
    static const char *list="|52772|52774|52829|52879|52933|52935|53017|53057|53067|53083|53093|53116|53308|53336|53342|53454|53490|53494|53535|";
    char token[64];return id && snprintf(token,sizeof(token),"|%s|",id)<(int)sizeof(token) && strstr(list,token);
}
static cJSON *convert(const cJSON *src){
    cJSON *dst=cJSON_CreateObject(),*ings=cJSON_CreateArray();if(!dst||!ings)die("JSON memory");
    const char *id=get(src,"idMeal");if(id){char *e;double x=strtod(id,&e);
        if(e!=id && !*e && isfinite(x) && x>=0){if(!cJSON_AddNumberToObject(dst,"id",x))die("JSON id");}
        else put(dst,"id",id);
    }
    const char *old[]={"strMeal","strMealAlternate","strCategory","strArea","strCountry"};
    const char *new[]={"name","alternateName","category","area","country"};
    for(size_t i=0;i<5;i++)move(dst,src,old[i],new[i]);
    const char *tags=get(src,"strTags");if(tags && *tags){
        cJSON *arr=cJSON_CreateArray();if(!arr)die("JSON tags");
        for(const char *p=tags;*p;){const char *q=strchr(p,',');if(!q)q=p+strlen(p);
            char s[N];trim(s,sizeof(s),p,q);
            if(*s && !cJSON_AddItemToArray(arr,cJSON_CreateString(s)))die("JSON tags");
            p=*q?q+1:q;
        }
        if(cJSON_GetArraySize(arr)){if(!cJSON_AddItemToObject(dst,"tags",arr))die("JSON tags");}
        else cJSON_Delete(arr);
    }
    for(int i=1;i<=20;i++){
        char k[40];snprintf(k,sizeof(k),"strIngredient%d",i);
        const char *name=get(src,k);if(!name || !*ws(name))continue;
        cJSON *ing=cJSON_CreateObject();if(!ing)die("JSON ingredient");put(ing,"name",name);
        snprintf(k,sizeof(k),"strMeasure%d",i);Parsed p=parse(get(src,k));
        if(p.has_amount && !cJSON_AddNumberToObject(ing,"amount",p.amount))die("JSON amount");
        put(ing,"unit",p.unit);put(ing,"note",p.note);
        if(!cJSON_AddItemToArray(ings,ing))die("JSON ingredients");
    }
    if(!cJSON_AddItemToObject(dst,"ingredients",ings))die("JSON ingredients");
    const char *text=get(src,"strInstructions");if(text && *text){char *s=instructions(text);put(dst,"instructions",s);free(s);}
    const char *fields[][2]={{"strMealThumb","image"},{"strYoutube","youtube"},{"strSource","source"},
        {"strImageSource","imageSource"},{"strCreativeCommonsConfirmed","creativeCommonsConfirmed"},{"dateModified","dateModified"}};
    for(size_t i=0;i<sizeof(fields)/sizeof(*fields);i++)move(dst,src,fields[i][0],fields[i][1]);
    return dst;
}
static void print_json(FILE *f,const cJSON *obj){
    fputs("    {\n",f);int first=1;
    for(const cJSON *v=obj->child;v;v=v->next){
        if(!first)fputs(",\n",f);
        first=0;
        cJSON *k=cJSON_CreateString(v->string);char *key=cJSON_PrintUnformatted(k);cJSON_Delete(k);
        if(!key)die("JSON key");
        fprintf(f,"      %s: ",key);cJSON_free(key);
        if(!strcmp(v->string,"ingredients")){
            fputs("[\n",f);int first_ing=1;
            for(const cJSON *ing=v->child;ing;ing=ing->next){
                char *s=cJSON_PrintUnformatted(ing);if(!s)die("JSON print");
                fprintf(f,"%s        %s",first_ing?"":",\n",s);first_ing=0;cJSON_free(s);
            }fputs("\n      ]",f);
        }else{char *s=cJSON_PrintUnformatted(v);if(!s)die("JSON print");fputs(s,f);cJSON_free(s);}
    }fputs("\n    }",f);
}
int main(void){
    FILE *f=fopen(INPUT,"rb");if(!f)die("Cannot open " INPUT);
    if(fseek(f,0,SEEK_END))die("Input seek");
    long len=ftell(f);if(len<0)die("Input length");rewind(f);
    char *s=alloc((size_t)len+1);if(fread(s,1,(size_t)len,f)!=(size_t)len)die("Input read");
    fclose(f);s[len]=0;cJSON *root=cJSON_ParseWithOpts(s,NULL,1);free(s);
    if(!root)die("Invalid JSON");
    cJSON *arr=cJSON_GetObjectItemCaseSensitive(root,"meals");
    if(!cJSON_IsArray(arr))die("Missing meals array");
    out=fopen(OUTPUT ".tmp","wb");if(!out)die("Cannot write output");
    fputs("{\n  \"meals\": [\n",out);int saved=0,skipped=0;
    for(const cJSON *meal=arr->child;meal;meal=meal->next){
        if(!cJSON_IsObject(meal))die("Invalid recipe");
        if(excluded(get(meal,"idMeal"))){skipped++;continue;}
        cJSON *result=convert(meal);if(saved)fputs(",\n",out);
        print_json(out,result);cJSON_Delete(result);saved++;
    }
    fputs("\n  ]\n}\n",out);int error=ferror(out);if(fclose(out)||error)die("Write error");out=NULL;
#ifdef _WIN32
    remove(OUTPUT);
#endif
    if(rename(OUTPUT ".tmp",OUTPUT))die("Could not save output");
    cJSON_Delete(root);
    printf("Saved %d recipes to %s (%d excluded).\n",saved,OUTPUT,skipped);
    return 0;
}

/* Include cJSON implementation so no separate compilation unit is needed. */
#include "cJSON.c"