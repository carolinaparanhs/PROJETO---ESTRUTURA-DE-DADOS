// btreeS.c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

2#ifdef _WIN32
  #include <direct.h>   // _mkdir
  #include <sys/stat.h> // _stat
#else
  #include <sys/stat.h>
  #include <sys/types.h>
  #include <unistd.h>
#endif

#define ORDEM 3
#define PASTA "dados_btree"
#define ARQUIVO "dados_btree/btree.bin"

typedef struct BTreeNode {
    int n;
    int folha;
    int chaves[2*ORDEM - 1];
    struct BTreeNode* filhos[2*ORDEM];
} BTreeNode;

typedef struct {
    BTreeNode* raiz;
} BTree;

static BTree* arvore = NULL;

static BTreeNode* criarNo(int folha) {
    BTreeNode* no = (BTreeNode*) malloc(sizeof(BTreeNode));
    if (!no) { fprintf(stderr, "malloc failed\n"); exit(1); }
    no->folha = folha;
    no->n = 0;
    for (int i = 0; i < 2*ORDEM; i++) no->filhos[i] = NULL;
    return no;
}

static BTree* criarBTree() {
    BTree* t = (BTree*) malloc(sizeof(BTree));
    if (!t) { fprintf(stderr, "malloc failed\n"); exit(1); }
    t->raiz = criarNo(1);
    return t;
}

static void dividirFilho(BTreeNode* x, int i, BTreeNode* y) {
    BTreeNode* z = criarNo(y->folha);
    z->n = ORDEM - 1;

    for (int j = 0; j < ORDEM-1; j++)
        z->chaves[j] = y->chaves[j+ORDEM];

    if (!y->folha) {
        for (int j = 0; j < ORDEM; j++)
            z->filhos[j] = y->filhos[j+ORDEM];
    }

    y->n = ORDEM - 1;

    for (int j = x->n; j >= i+1; j--)
        x->filhos[j+1] = x->filhos[j];
    x->filhos[i+1] = z;

    for (int j = x->n-1; j >= i; j--)
        x->chaves[j+1] = x->chaves[j];
    x->chaves[i] = y->chaves[ORDEM-1];
    x->n++;
}

static void inserirNaoCheio(BTreeNode* x, int k) {
    int i = x->n - 1;
    if (x->folha) {
        while (i >= 0 && k < x->chaves[i]) {
            x->chaves[i+1] = x->chaves[i];
            i--;
        }
        x->chaves[i+1] = k;
        x->n++;
    } else {
        while (i >= 0 && k < x->chaves[i]) i--;
        i++;
        if (x->filhos[i]->n == 2*ORDEM - 1) {
            dividirFilho(x, i, x->filhos[i]);
            if (k > x->chaves[i]) i++;
        }
        inserirNaoCheio(x->filhos[i], k);
    }
}

static void inserirBTree(BTree* T, int k) {
    BTreeNode* r = T->raiz;
    if (r->n == 2*ORDEM - 1) {
        BTreeNode* s = criarNo(0);
        T->raiz = s;
        s->filhos[0] = r;
        dividirFilho(s, 0, r);
        inserirNaoCheio(s, k);
    } else {
        inserirNaoCheio(r, k);
    }
}

static int buscarNodo(BTreeNode* x, int k) {
    int i = 0;
    while (i < x->n && k > x->chaves[i]) i++;
    if (i < x->n && k == x->chaves[i]) return 1;
    if (x->folha) return 0;
    return buscarNodo(x->filhos[i], k);
}

static int buscarBTree(BTree* T, int k) {
    return buscarNodo(T->raiz, k);
}

/* ---------- Persistência ---------- */

static void criarPastaSeNaoExistir(const char* caminho) {
#ifdef _WIN32
    struct _stat info;
    if (_stat(caminho, &info) != 0) {
        if (_mkdir(caminho) != 0) {
            fprintf(stderr, "Erro ao criar a pasta %s: %d\n", caminho, errno);
            return;
        }
    }
#else
    struct stat info;
    if (stat(caminho, &info) != 0) {
        if (mkdir(caminho, 0777) != 0 && errno != EEXIST) {
            fprintf(stderr, "Erro ao criar a pasta %s: %d\n", caminho, errno);
            return;
        }
    }
#endif
}

static void percorrerEEscrever(BTreeNode* nodo, FILE* fp) {
    int i;
    for (i = 0; i < nodo->n; i++) {
        if (!nodo->folha) percorrerEEscrever(nodo->filhos[i], fp);
        fwrite(&nodo->chaves[i], sizeof(int), 1, fp);
    }
    if (!nodo->folha) percorrerEEscrever(nodo->filhos[i], fp);
}

static void salvarBTree(BTree* T) {
    criarPastaSeNaoExistir(PASTA);
    FILE* fp = fopen(ARQUIVO, "wb");
    if (!fp) {
        fprintf(stderr, "Erro ao abrir o arquivo %s para escrita: %s\n", ARQUIVO, strerror(errno));
        return;
    }
    if (T && T->raiz) percorrerEEscrever(T->raiz, fp);
    fclose(fp);
}

static void carregarBTree(BTree* T) {
    FILE* fp = fopen(ARQUIVO, "rb");
    if (!fp) return; // arquivo não existe ainda

    int key;
    while (fread(&key, sizeof(int), 1, fp) == 1) {
        inserirBTree(T, key);
    }
    fclose(fp);
}

/* ---------- Exportadas para JNA ---------- */

#ifdef _WIN32
  #define EXPORT __declspec(dllexport)
#else
  #define EXPORT
#endif

EXPORT void btree_insert(int key) {
    if (!arvore) {
        arvore = criarBTree();
        carregarBTree(arvore);
    }
    // debug
    printf("[btree_insert] Inserindo %d\n", key);
    fflush(stdout);

    inserirBTree(arvore, key);
    salvarBTree(arvore);
}

EXPORT int btree_search(int key) {
    if (!arvore) {
        arvore = criarBTree();
        carregarBTree(arvore);
    }
    // debug
    printf("[btree_search] Buscando %d\n", key);
    fflush(stdout);

    return buscarBTree(arvore, key);
}


