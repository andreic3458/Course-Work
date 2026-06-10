#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* ── Disjoint-Set: union by rank + path compression ── */
typedef struct { int *parent, *rnk, n; } UandF;

UandF *uandf(int n) {
    UandF *ds  = malloc(sizeof(UandF));
    ds->parent = malloc((n+1)*sizeof(int));
    ds->rnk    = calloc((n+1), sizeof(int));
    ds->n = n;
    for (int i=1; i<=n; i++) ds->parent[i]=i;
    return ds;
}

void make_set(UandF *ds, int i)  { ds->parent[i]=i; ds->rnk[i]=0; }

int find_set(UandF *ds, int i) {
    if (ds->parent[i]!=i) ds->parent[i]=find_set(ds,ds->parent[i]);
    return ds->parent[i];
}

void union_sets(UandF *ds, int i, int j) {
    int ri=find_set(ds,i), rj=find_set(ds,j);
    if (ri==rj) return;
    if      (ds->rnk[ri]<ds->rnk[rj]) ds->parent[ri]=rj;
    else if (ds->rnk[ri]>ds->rnk[rj]) ds->parent[rj]=ri;
    else { ds->parent[rj]=ri; ds->rnk[ri]++; }
}

/* Counts unique roots among fg pixels; fills remap[root]->1..num_sets */
int final_sets(UandF *ds, int *fg, int nfg, int *remap_out) {
    int count=0;
    for (int i=0; i<nfg; i++) {
        int r=find_set(ds,fg[i]);
        if (remap_out[r]==0) remap_out[r]=++count;
    }
    return count;
}

void free_uandf(UandF *ds) { free(ds->parent); free(ds->rnk); free(ds); }

/* component id -> letter a-z (problem says < 26 components) */
static char label_char(int id) {
    return (char)('a' + (id-1));
}

#define MAX_ROWS 1000
#define MAX_COLS 1000

static char grid[MAX_ROWS][MAX_COLS+2];
static int  rlen[MAX_ROWS];
static int  lbl [MAX_ROWS][MAX_COLS];

static int *g_size;
static int cmp_desc(const void *a, const void *b){
    return g_size[*(const int*)b]-g_size[*(const int*)a];
}

/* is pixel (r,c) foreground? */
static inline int is_fg(int r, int c) {
    if (c >= rlen[r]) return 0;
    return grid[r][c] == '+';
}

int main(void) {
    int rows=0, cols=0;

    while (fgets(grid[rows], sizeof(grid[rows]), stdin)) {
        /* strip newline but keep spaces (they are background pixels) */
        int len=(int)strlen(grid[rows]);
        while (len>0 && (grid[rows][len-1]=='\n'||grid[rows][len-1]=='\r'))
            grid[rows][--len]='\0';
        rlen[rows]=len;
        if (len>cols) cols=len;
        if (++rows>=MAX_ROWS) break;
    }

    /* 1. Input image */
    printf("1. Input binary image:\n");
    for (int r=0; r<rows; r++) printf("%s\n",grid[r]);
    printf("\n");

    /* collect foreground pixels and build disjoint-set */
    int  nfg=0;
    int *fg = malloc(rows*cols*sizeof(int));
    UandF *ds = uandf(rows*cols);

    for (int r=0; r<rows; r++)
        for (int c=0; c<rlen[r]; c++) {
            if (!is_fg(r,c)) continue;
            int me=r*cols+c+1;
            fg[nfg++]=me;
            if (c>0 && is_fg(r,c-1))
                union_sets(ds, me, r*cols+(c-1)+1);
            if (r>0 && is_fg(r-1,c))
                union_sets(ds, me, (r-1)*cols+c+1);
        }

    int *remap    = calloc((ds->n+1), sizeof(int));
    int  num_sets = final_sets(ds, fg, nfg, remap);
    free(fg);

    /* build label grid */
    for (int r=0; r<rows; r++)
        for (int c=0; c<rlen[r]; c++)
            lbl[r][c] = is_fg(r,c) ? remap[find_set(ds,r*cols+c+1)] : 0;

    /* sizes */
    int *size = calloc((num_sets+1), sizeof(int));
    for (int r=0; r<rows; r++)
        for (int c=0; c<rlen[r]; c++)
            if (lbl[r][c]) size[lbl[r][c]]++;

    /* order by size desc */
    int *order = malloc((num_sets+1)*sizeof(int));
    int  ncomp=0;
    for (int i=1; i<=num_sets; i++) if (size[i]>0) order[ncomp++]=i;
    g_size=size;
    qsort(order, ncomp, sizeof(int), cmp_desc);

    /* 2. All components labelled */
    printf("2. Connected component image (all components):\n");
    for (int r=0; r<rows; r++) {
        for (int c=0; c<cols; c++) {
            if (c >= rlen[r] || !is_fg(r,c)) putchar(' ');
            else putchar(label_char(lbl[r][c]));
        }
        putchar('\n');
    }
    printf("\n");

    /* 3. Sorted list */
    printf("3. Component list sorted by size (descending):\n");
    printf("   Size  Label\n");
    for (int i=0; i<ncomp; i++)
        printf("   %-5d %c\n", size[order[i]], label_char(order[i]));
    printf("\n");

    /* 4. Size > 1 */
    printf("4. Connected component image (size > 1):\n");
    for (int r=0; r<rows; r++) {
        for (int c=0; c<cols; c++) {
            int lb = (c<rlen[r]) ? lbl[r][c] : 0;
            if (lb && size[lb]>1) putchar(label_char(lb));
            else putchar(' ');
        }
        putchar('\n');
    }
    printf("\n");

    /* 5. Size > 4 */
    printf("5. Connected component image (size > 4):\n");
    for (int r=0; r<rows; r++) {
        for (int c=0; c<cols; c++) {
            int lb = (c<rlen[r]) ? lbl[r][c] : 0;
            if (lb && size[lb]>4) putchar(label_char(lb));
            else putchar(' ');
        }
        putchar('\n');
    }
    printf("\n");

    free(remap); free(order); free(size); free_uandf(ds);
    return 0;
}
