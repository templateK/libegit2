#include <stdlib.h>
#include <assert.h>

#include "emacs-module.h"
#include "git2.h"

#ifdef EGIT_DEBUG
#include "egit-debug.h"
#endif

#include "interface.h"
#include "egit-commit.h"
#include "egit-describe.h"
#include "egit-diff.h"
#include "egit-libgit2.h"
#include "egit-message.h"
#include "egit-object.h"
#include "egit-tree.h"
#include "egit-treebuilder.h"
#include "egit-reference.h"
#include "egit-repository.h"
#include "egit-revparse.h"
#include "egit-revwalk.h"
#include "egit.h"

egit_type egit_get_type(emacs_env *env, emacs_value _obj)
{
    if (!em_user_ptrp(env, _obj))
        return EGIT_UNKNOWN;
    egit_object *obj = (egit_object*) EM_EXTRACT_USER_PTR(_obj);
    return obj->type;
}

bool egit_assert_type(emacs_env *env, emacs_value obj, egit_type type, emacs_value predicate)
{
    if (type == egit_get_type(env, obj))
        return true;
    em_signal_wrong_type(env, predicate, obj);
    return false;
}

bool egit_assert_object(emacs_env *env, emacs_value obj)
{
    egit_type type = egit_get_type(env, obj);
    if (type == EGIT_COMMIT || type == EGIT_TREE ||
        type == EGIT_BLOB || type == EGIT_TAG || type == EGIT_OBJECT)
        return true;
    em_signal_wrong_type(env, esym_libgit_object_p, obj);
    return false;
}

ptrdiff_t egit_assert_list(emacs_env *env, egit_type type, emacs_value predicate, emacs_value arg)
{
    ptrdiff_t nelems = 0;

    while (em_consp(env, arg)) {
        emacs_value car = em_car(env, arg);
        if (!egit_assert_type(env, car, type, predicate))
            return -1;
        nelems++;
        arg = em_cdr(env, arg);
    }

    if (EM_EXTRACT_BOOLEAN(arg)) {
        em_signal_wrong_type(env, esym_listp, arg);
        return -1;
    }

    return nelems;
}

void egit_finalize(void* _obj)
{
#ifdef EGIT_DEBUG
    egit_signal_finalize(_obj);
#endif

    // The argument type must be void* to make this function work as an Emacs finalizer
    egit_object *obj = (egit_object*)_obj;
    egit_object *parent = obj->parent;

    // For reference-counted types, decref and possibly abort
    switch (obj->type) {
    case EGIT_REPOSITORY:
        obj->refcount--;
        if (obj->refcount != 0)
            return;
    default: break;
    }

#ifdef EGIT_DEBUG
    egit_signal_free(_obj);
#endif

    // Free the object based on its type
    // For types that only expose weak pointers to the parent, this should be a no-op
    switch (obj->type) {
    case EGIT_COMMIT:
    case EGIT_TREE:
    case EGIT_OBJECT: git_object_free(obj->ptr); break;
    case EGIT_DIFF: git_diff_free(obj->ptr); break;
    case EGIT_REPOSITORY: git_repository_free(obj->ptr); break;
    case EGIT_REFERENCE: git_reference_free(obj->ptr); break;
    case EGIT_REVWALK: git_revwalk_free(obj->ptr); break;
    case EGIT_TREEBUILDER: git_treebuilder_free(obj->ptr); break;
    default: break;
    }

    // Free the wrapper, then release the reference to the parent, if applicable
    free(obj);
    if (parent)
        egit_finalize(parent);
}

emacs_value egit_wrap(emacs_env *env, egit_type type, const void* data, egit_object *parent)
{
    // If it's a git_object, try to be more specific
    if (type == EGIT_OBJECT) {
        switch (git_object_type(data)) {
        case GIT_OBJ_COMMIT: type = EGIT_COMMIT; break;
        case GIT_OBJ_TREE: type = EGIT_TREE; break;
        case GIT_OBJ_BLOB: type = EGIT_BLOB; break;
        case GIT_OBJ_TAG: type = EGIT_TAG; break;
        default: break;
        }
    }

    // Increase refcounts of owner object(s), if applicable
    if (parent)
        parent->refcount++;

    egit_object *wrapper;
    wrapper = (egit_object*) malloc(sizeof(egit_object));
    wrapper->type = type;
    wrapper->ptr = (void*) data;
    wrapper->parent = parent;

    // This has no effect for types that are not reference-counted
    wrapper->refcount = 1;

#ifdef EGIT_DEBUG
    egit_signal_alloc(wrapper);
#endif

    return EM_USER_PTR(wrapper, egit_finalize);
}

typedef emacs_value (*func_0)(emacs_env*);
typedef emacs_value (*func_1)(emacs_env*, emacs_value);
typedef emacs_value (*func_2)(emacs_env*, emacs_value, emacs_value);
typedef emacs_value (*func_3)(emacs_env*, emacs_value, emacs_value, emacs_value);
typedef emacs_value (*func_4)(emacs_env*, emacs_value, emacs_value, emacs_value,
                              emacs_value);
typedef emacs_value (*func_5)(emacs_env*, emacs_value, emacs_value, emacs_value,
                              emacs_value, emacs_value);
typedef emacs_value (*func_6)(emacs_env*, emacs_value, emacs_value, emacs_value,
                              emacs_value, emacs_value, emacs_value);
typedef emacs_value (*func_7)(emacs_env*, emacs_value, emacs_value, emacs_value,
                              emacs_value, emacs_value, emacs_value, emacs_value);

// Get an argument index, or nil. Useful for simulating optional arguments.
#define GET_SAFE(arglist, nargs, index) ((index) < (nargs) ? (arglist)[(index)] : esym_nil)

static emacs_value egit_dispatch_0(emacs_env *env, __attribute__((unused)) ptrdiff_t nargs,
                            __attribute__((unused)) emacs_value *args, void *data)
{
    func_0 func = (func_0) data;
    return func(env);
}

static emacs_value egit_dispatch_1(emacs_env *env, ptrdiff_t nargs, emacs_value *args, void *data)
{
    func_1 func = (func_1) data;
    return func(env, GET_SAFE(args, nargs, 0));
}

static emacs_value egit_dispatch_2(emacs_env *env, ptrdiff_t nargs, emacs_value *args, void *data)
{
    func_2 func = (func_2) data;
    return func(env, GET_SAFE(args, nargs, 0), GET_SAFE(args, nargs, 1));
}

static emacs_value egit_dispatch_3(emacs_env *env, ptrdiff_t nargs, emacs_value *args, void *data)
{
    func_3 func = (func_3) data;
    return func(env, GET_SAFE(args, nargs, 0), GET_SAFE(args, nargs, 1), GET_SAFE(args, nargs, 2));
}

static emacs_value egit_dispatch_4(emacs_env *env, ptrdiff_t nargs, emacs_value *args, void *data)
{
    func_4 func = (func_4) data;
    return func(env, GET_SAFE(args, nargs, 0), GET_SAFE(args, nargs, 1), GET_SAFE(args, nargs, 2),
                GET_SAFE(args, nargs, 3));
}

static emacs_value egit_dispatch_5(emacs_env *env, ptrdiff_t nargs, emacs_value *args, void *data)
{
    func_5 func = (func_5) data;
    return func(env, GET_SAFE(args, nargs, 0), GET_SAFE(args, nargs, 1), GET_SAFE(args, nargs, 2),
                GET_SAFE(args, nargs, 3), GET_SAFE(args, nargs, 4));
}

static emacs_value egit_dispatch_6(emacs_env *env, ptrdiff_t nargs, emacs_value *args, void *data)
{
    func_6 func = (func_6) data;
    return func(env, GET_SAFE(args, nargs, 0), GET_SAFE(args, nargs, 1), GET_SAFE(args, nargs, 2),
                GET_SAFE(args, nargs, 3), GET_SAFE(args, nargs, 4), GET_SAFE(args, nargs, 5));
}

static emacs_value egit_dispatch_7(emacs_env *env, ptrdiff_t nargs, emacs_value *args, void *data)
{
    func_7 func = (func_7) data;
    return func(env, GET_SAFE(args, nargs, 0), GET_SAFE(args, nargs, 1), GET_SAFE(args, nargs, 2),
                GET_SAFE(args, nargs, 3), GET_SAFE(args, nargs, 4), GET_SAFE(args, nargs, 5),
                GET_SAFE(args, nargs, 6));
}

bool egit_dispatch_error(emacs_env *env, int retval)
{
    if (retval >= 0) return false;

    const git_error *err = giterr_last();
    if (!err) return false;

    emacs_value error = em_findenum_error(err->klass);
    if (!EM_EXTRACT_BOOLEAN(error))
        error = esym_giterr;

    em_signal(env, error, err->message);
    return true;
}

EGIT_DOC(typeof, "OBJ", "Return the type of the git pointer OBJ, or nil.");
static emacs_value egit_typeof(emacs_env *env, emacs_value val)
{
    switch (egit_get_type(env, val)) {
    case EGIT_REPOSITORY: return esym_repository;
    case EGIT_REFERENCE: return esym_reference;
    case EGIT_COMMIT: return esym_commit;
    case EGIT_TREE: return esym_tree;
    case EGIT_OBJECT: return esym_object;
    case EGIT_DIFF: return esym_diff;
    case EGIT_DIFF_DELTA: return esym_diff_delta;
    case EGIT_DIFF_BINARY: return esym_diff_binary;
    case EGIT_DIFF_HUNK: return esym_diff_hunk;
    case EGIT_DIFF_LINE: return esym_diff_line;
    case EGIT_REFSPEC: return esym_refspec;
    case EGIT_REVWALK: return esym_revwalk;
    case EGIT_TREEBUILDER: return esym_treebuilder;
    default: return esym_nil;
    }
}

#define TYPECHECKER(caps, small, text)                                  \
    EGIT_DOC(small##_p, "OBJ", "Return non-nil if OBJ is a git " text "."); \
    static emacs_value egit_##small##_p(emacs_env *env, emacs_value obj)\
    {                                                                   \
        return egit_get_type(env, obj) == EGIT_##caps ? esym_t : esym_nil;  \
    }

TYPECHECKER(ANNOTATED_COMMIT, annotated_commit, "annotated commit");
TYPECHECKER(BLAME, blame, "blame");
TYPECHECKER(BLAME_HUNK, blame_hunk, "blame hunk");
TYPECHECKER(COMMIT, commit, "commit");
TYPECHECKER(BLOB, blob, "blob");
TYPECHECKER(CONFIG, config, "config");
TYPECHECKER(CRED, cred, "credential");
TYPECHECKER(DIFF, diff, "diff");
TYPECHECKER(DIFF_DELTA, diff_delta, "diff delta");
TYPECHECKER(DIFF_BINARY, diff_binary, "diff binary");
TYPECHECKER(DIFF_HUNK, diff_hunk, "diff hunk");
TYPECHECKER(DIFF_LINE, diff_line, "diff line");
TYPECHECKER(INDEX, index, "index.");
TYPECHECKER(INDEX_ENTRY, index_entry, "index entry");
TYPECHECKER(PATHSPEC, pathspec, "pathspec");
TYPECHECKER(PATHSPEC_MATCH_LIST, pathspec_match_list, "pathspec match list");
TYPECHECKER(REFERENCE, reference, "reference");
TYPECHECKER(REFLOG, reflog, "reflog");
TYPECHECKER(REFLOG_ENTRY, reflog_entry, "reflog entry");
TYPECHECKER(REFSPEC, refspec, "refspec");
TYPECHECKER(REMOTE, remote, "remote");
TYPECHECKER(REPOSITORY, repository, "repository");
TYPECHECKER(REVWALK, revwalk, "repository");
TYPECHECKER(SIGNATURE, signature, "signature");
TYPECHECKER(SUBMODULE, submodule, "submodule");
TYPECHECKER(TAG, tag, "tag");
TYPECHECKER(TRANSACTION, transaction, "transaction");
TYPECHECKER(TREE, tree, "tree");
TYPECHECKER(TREEBUILDER, treebuilder, "treebuilder");

#undef TYPECHECKER

EGIT_DOC(object_p, "OBJ", "Return non-nil if OBJ is a git object.");
static emacs_value egit_object_p(emacs_env *env, emacs_value obj)
{
    egit_type type = egit_get_type(env, obj);
    return (type == EGIT_COMMIT || type == EGIT_TREE || type == EGIT_BLOB ||
            type == EGIT_TAG || type == EGIT_OBJECT) ? esym_t : esym_nil;
}

#define DEFUN(ename, cname, min_nargs, max_nargs)                       \
    em_defun(env, (ename),                                              \
             env->make_function(                                        \
                 env, (min_nargs), (max_nargs),                         \
                 egit_dispatch_##max_nargs,                             \
                 egit_##cname##__doc,                                   \
                 egit_##cname))

void egit_init(emacs_env *env)
{
    // Debug mode functions
#ifdef EGIT_DEBUG
    DEFUN("libgit--allocs", _allocs, 0, 0);
    DEFUN("libgit--finalizes", _finalizes, 0, 0);
    DEFUN("libgit--frees", _frees, 0, 0);
    DEFUN("libgit--refcount", _refcount, 1, 1);
    DEFUN("libgit--wrapper", _wrapper, 1, 1);
    DEFUN("libgit--wrapped", _wrapped, 1, 1);
    DEFUN("libgit--parent-wrapper", _parent_wrapper, 1, 1);
#endif

    // Type checkers
    DEFUN("libgit-typeof", typeof, 1, 1);
    DEFUN("libgit-commit-p", commit_p, 1, 1);
    DEFUN("libgit-diff-p", diff_p, 1, 1);
    DEFUN("libgit-diff-delta-p", diff_delta_p, 1, 1);
    DEFUN("libgit-diff-binary-p", diff_binary_p, 1, 1);
    DEFUN("libgit-diff-hunk-p", diff_hunk_p, 1, 1);
    DEFUN("libgit-diff-line-p", diff_line_p, 1, 1);
    DEFUN("libgit-object-p", object_p, 1, 1);
    DEFUN("libgit-reference-p", reference_p, 1, 1);
    DEFUN("libgit-repository-p", repository_p, 1, 1);
    DEFUN("libgit-revwalk-p", revwalk_p, 1, 1);
    DEFUN("libgit-tree-p", tree_p, 1, 1);
    DEFUN("libgit-treebuilder-p", treebuilder_p, 1, 1);

    // Libgit2 (not namespaced as others!)
    DEFUN("libgit-feature-p", libgit2_feature_p, 1, 1);
    DEFUN("libgit-version", libgit2_version, 0, 0);

    // Commit
    DEFUN("libgit-commit-lookup", commit_lookup, 2, 2);
    DEFUN("libgit-commit-lookup-prefix", commit_lookup_prefix, 2, 2);

    DEFUN("libgit-commit-author", commit_author, 1, 1);
    DEFUN("libgit-commit-body", commit_body, 1, 1);
    DEFUN("libgit-commit-committer", commit_committer, 1, 1);
    DEFUN("libgit-commit-id", commit_id, 1, 1);
    DEFUN("libgit-commit-message", commit_message, 1, 1);
    DEFUN("libgit-commit-nth-gen-ancestor", commit_nth_gen_ancestor, 2, 2);
    DEFUN("libgit-commit-owner", commit_owner, 1, 1);
    DEFUN("libgit-commit-parent", commit_parent, 1, 2);
    DEFUN("libgit-commit-parent-id", commit_parent_id, 1, 2);
    DEFUN("libgit-commit-parentcount", commit_parentcount, 1, 1);
    DEFUN("libgit-commit-summary", commit_summary, 1, 1);
    DEFUN("libgit-commit-time", commit_time, 1, 1);
    DEFUN("libgit-commit-tree", commit_tree, 1, 1);
    DEFUN("libgit-commit-tree-id", commit_tree_id, 1, 1);

    DEFUN("libgit-commit-create", commit_create, 6, 7);

    // Describe
    DEFUN("libgit-describe-commit", describe_commit, 1, 2);
    DEFUN("libgit-describe-workdir", describe_workdir, 1, 2);

    // Diff
    DEFUN("libgit-diff-index-to-index", diff_index_to_index, 3, 4);
    DEFUN("libgit-diff-index-to-workdir", diff_index_to_workdir, 1, 3);
    DEFUN("libgit-diff-tree-to-index", diff_tree_to_index, 1, 4);
    DEFUN("libgit-diff-tree-to-tree", diff_tree_to_tree, 1, 4);
    DEFUN("libgit-diff-tree-to-workdir", diff_tree_to_workdir, 1, 3);
    DEFUN("libgit-diff-tree-to-workdir-with-index", diff_tree_to_workdir_with_index, 1, 3);
    DEFUN("libgit-diff-find-similar", diff_find_similar, 1, 2);

    DEFUN("libgit-diff-foreach", diff_foreach, 2, 5);
    DEFUN("libgit-diff-print", diff_print, 1, 3);

    DEFUN("libgit-diff-delta-file-id", diff_delta_file_id, 1, 2);
    DEFUN("libgit-diff-delta-file-path", diff_delta_file_path, 1, 2);
    DEFUN("libgit-diff-delta-nfiles", diff_delta_nfiles, 1, 1);
    DEFUN("libgit-diff-delta-similarity", diff_delta_similarity, 1, 1);
    DEFUN("libgit-diff-delta-status", diff_delta_status, 1, 1);
    DEFUN("libgit-diff-delta-file-exists-p", diff_delta_file_exists_p, 1, 2);

    DEFUN("libgit-diff-hunk-header", diff_hunk_header, 1, 1);
    DEFUN("libgit-diff-hunk-lines", diff_hunk_lines, 1, 2);
    DEFUN("libgit-diff-hunk-start", diff_hunk_start, 1, 2);

    DEFUN("libgit-diff-line-origin", diff_line_origin, 1, 1);
    DEFUN("libgit-diff-line-lineno", diff_line_lineno, 2, 2);
    DEFUN("libgit-diff-line-content", diff_line_content, 1, 1);

    DEFUN("libgit-diff-get-delta", diff_get_delta, 2, 2);
    DEFUN("libgit-diff-num-deltas", diff_num_deltas, 1, 2);

    // Message
    DEFUN("libgit-message-prettify", message_prettify, 1, 2);
    DEFUN("libgit-message-trailers", message_trailers, 1, 1);

    // Object
    DEFUN("libgit-object-lookup", object_lookup, 2, 3);
    DEFUN("libgit-object-lookup-prefix", object_id, 2, 3);

    DEFUN("libgit-object-id", object_id, 1, 1);
    DEFUN("libgit-object-owner", object_owner, 1, 1);
    DEFUN("libgit-object-short-id", object_short_id, 1, 1);

    // Reference
    DEFUN("libgit-reference-create", reference_create, 3, 5);
    DEFUN("libgit-reference-create-matching", reference_create_matching, 3, 6);
    DEFUN("libgit-reference-dup", reference_dup, 1, 1);
    DEFUN("libgit-reference-dwim", reference_dwim, 2, 2);
    DEFUN("libgit-reference-lookup", reference_lookup, 2, 2);

    DEFUN("libgit-reference-list", reference_list, 1, 1);
    DEFUN("libgit-reference-name", reference_name, 1, 1);
    DEFUN("libgit-reference-owner", reference_owner, 1, 1);
    DEFUN("libgit-reference-peel", reference_peel, 1, 2);
    DEFUN("libgit-reference-resolve", reference_resolve, 1, 1);
    DEFUN("libgit-reference-shorthand", reference_shorthand, 1, 1);
    DEFUN("libgit-reference-symbolic-target", reference_symbolic_target, 1, 1);
    DEFUN("libgit-reference-target", reference_target, 1, 1);
    DEFUN("libgit-reference-target-peel", reference_target, 1, 1);
    DEFUN("libgit-reference-type", reference_type, 1, 1);

    DEFUN("libgit-reference-delete", reference_delete, 1, 1);
    DEFUN("libgit-reference-ensure-log", reference_ensure_log, 2, 2);
    DEFUN("libgit-reference-remove", reference_delete, 2, 2);

    DEFUN("libgit-reference-branch-p", reference_branch_p, 1, 1);
    DEFUN("libgit-reference-direct-p", reference_direct_p, 1, 1);
    DEFUN("libgit-reference-has-log-p", reference_has_log_p, 2, 2);
    DEFUN("libgit-reference-name-to-id", reference_name_to_id, 2, 2);
    DEFUN("libgit-reference-note-p", reference_note_p, 1, 1);
    DEFUN("libgit-reference-remote-p", reference_remote_p, 1, 1);
    DEFUN("libgit-reference-symbolic-p", reference_symbolic_p, 1, 1);
    DEFUN("libgit-reference-tag-p", reference_tag_p, 1, 1);
    DEFUN("libgit-reference-valid-name-p", reference_valid_name_p, 1, 1);

    DEFUN("libgit-reference-foreach", reference_foreach, 2, 2);
    DEFUN("libgit-reference-foreach-glob", reference_foreach_glob, 3, 3);
    DEFUN("libgit-reference-foreach-name", reference_foreach_name, 2, 2);

    // Repository
    DEFUN("libgit-repository-init", repository_init, 1, 2);
    DEFUN("libgit-repository-open", repository_open, 1, 1);
    DEFUN("libgit-repository-open-bare", repository_open_bare, 1, 1);

    DEFUN("libgit-repository-commondir", repository_commondir, 1, 1);
    DEFUN("libgit-repository-config", repository_config, 1, 1);
    DEFUN("libgit-repository-get-namespace", repository_get_namespace, 1, 1);
    DEFUN("libgit-repository-head", repository_head, 1, 1);
    DEFUN("libgit-repository-head-for-worktree", repository_head_for_worktree, 2, 2);
    DEFUN("libgit-repository-ident", repository_ident, 1, 1);
    DEFUN("libgit-repository-index", repository_index, 1, 1);
    DEFUN("libgit-repository-message", repository_message, 1, 1);
    DEFUN("libgit-repository-path", repository_path, 1, 1);
    DEFUN("libgit-repository-state", repository_state, 1, 1);
    DEFUN("libgit-repository-workdir", repository_workdir, 1, 1);

    DEFUN("libgit-repository-detach-head", repository_detach_head, 1, 1);
    DEFUN("libgit-repository-message-remove", repository_message_remove, 1, 1);
    DEFUN("libgit-repository-set-head", repository_set_head, 2, 2);
    DEFUN("libgit-repository-set-head-detached", repository_set_head_detached, 2, 2);
    DEFUN("libgit-repository-set-ident", repository_set_ident, 1, 3);
    DEFUN("libgit-repository-set-namespace", repository_set_namespace, 2, 2);
    DEFUN("libgit-repository-set-workdir", repository_set_workdir, 2, 3);
    DEFUN("libgit-repository-state-cleanup", repository_state_cleanup, 1, 1);

    DEFUN("libgit-repository-bare-p", repository_bare_p, 1, 1);
    DEFUN("libgit-repository-empty-p", repository_empty_p, 1, 1);
    DEFUN("libgit-repository-head-detached-p", repository_empty_p, 1, 1);
    DEFUN("libgit-repository-head-unborn-p", repository_empty_p, 1, 1);
    DEFUN("libgit-repository-shallow-p", repository_shallow_p, 1, 1);
    DEFUN("libgit-repository-worktree-p", repository_worktree_p, 1, 1);

    DEFUN("libgit-repository-discover", repository_discover, 0, 3);

    // Revparse
    DEFUN("libgit-revparse", revparse, 2, 2);
    DEFUN("libgit-revparse-ext", revparse_ext, 2, 2);
    DEFUN("libgit-revparse-single", revparse_single, 2, 2);

}
