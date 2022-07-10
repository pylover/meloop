#include "monad.h"

#include <stdio.h>
#include <stdlib.h>


struct monad_context {
    struct monad *monad;
    monad_success success;
    monad_failure fail;
};


struct monad {
    monad_task run;
    void *args;
    struct monad *next;
};


static void _run(struct monad_context *ctx, struct monad *m, void *data) {
    m->run(ctx, m->args, data);
}


void monad_run(struct monad *m, void *data, monad_success success, 
        monad_failure fail) {
    struct monad_context *ctx = malloc(sizeof(struct monad_context));
    ctx->monad = m;
    ctx->success = success;
    ctx->fail = fail;
    _run(ctx, m, data);
}


void monad_bind(struct monad *m1, struct monad *m2) {
    struct monad *last = m1;
    while (last->next != NULL) {
        last = last->next;
    }
    last->next = m2;
}


void monad_append(struct monad *m, monad_task task, void* args) {
    monad_bind(m, monad_return(task, args));
}


void monad_failed(struct monad_context* ctx, const char *reason) {
    if (ctx->fail != NULL) {
        ctx->fail(ctx, (char *)reason);
    }
    free(ctx);
}


void monad_succeeded(struct monad_context* ctx, void *result) {
    struct monad *next = ctx->monad->next;
    if (next == NULL) {
        if (ctx->success != NULL) {
            ctx->success(ctx, result);
        }
        free(ctx);
        return;
    }

    ctx->monad = next;
    _run(ctx, next, result);
}


void monad_pass(struct monad_context *ctx, void *args, void *data) {
    monad_succeeded(ctx, data);
}


struct monad * monad_return(monad_task task, void *args) {
    struct monad *m = malloc(sizeof(struct monad));
    m->run = task;
    m->args = args;
    m->next = NULL;
    return m;
};


struct monad * monad_new() {
    return monad_return(monad_pass, NULL); 
};


void monad_free(struct monad *m) {
    if (m == NULL) {
        return;
    }
    monad_free(m->next);
    free(m);
}
