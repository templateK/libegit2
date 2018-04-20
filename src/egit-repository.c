#include <string.h>

#include "git2.h"

#include "egit.h"
#include "interface.h"
#include "egit-repository.h"

emacs_value egit_repository_p(emacs_env *env, emacs_value obj)
{
    return egit_get_type(env, obj) == EGIT_REPOSITORY ? em_t : em_nil;
}

emacs_value egit_clone(emacs_env *env, emacs_value _url, emacs_value _path)
{
    EGIT_ASSERT_STRING(_url);
    EGIT_ASSERT_STRING(_path);

    git_repository *repo;
    int retval;
    {
        char *url = em_get_string(env, _url);
        char *path = em_get_string(env, _path);
        retval = git_clone(&repo, url, path, NULL);
        free(url);
        free(path);
    }
    EGIT_CHECK_ERROR(retval);

    return egit_wrap(env, EGIT_REPOSITORY, repo);
}

emacs_value egit_repository_init(emacs_env *env, emacs_value _path, emacs_value _is_bare)
{
    EGIT_ASSERT_STRING(_path);

    git_repository *repo;
    int retval;
    {
        char *path = em_get_string(env, _path);
        unsigned int is_bare = env->is_not_nil(env, _is_bare) ? 1 : 0;
        retval = git_repository_init(&repo, path, is_bare);
        free(path);
    }
    EGIT_CHECK_ERROR(retval);

    return egit_wrap(env, EGIT_REPOSITORY, repo);
}

emacs_value egit_repository_open(emacs_env *env, emacs_value _path)
{
    EGIT_ASSERT_STRING(_path);

    git_repository *repo;
    int retval;
    {
        char *path = em_get_string(env, _path);
        retval = git_repository_open(&repo, path);
        free(path);
    }
    EGIT_CHECK_ERROR(retval);

    return egit_wrap(env, EGIT_REPOSITORY, repo);
}

emacs_value egit_repository_path(emacs_env *env, emacs_value _repo)
{
    EGIT_ASSERT_REPOSITORY(_repo);
    git_repository *repo = EGIT_EXTRACT(_repo);
    const char *path = git_repository_path(repo);
    return env->make_string(env, path, strlen(path));
}

emacs_value egit_repository_workdir(emacs_env *env, emacs_value _repo)
{
    EGIT_ASSERT_REPOSITORY(_repo);
    git_repository *repo = EGIT_EXTRACT(_repo);
    const char *path = git_repository_workdir(repo);
    return path ? env->make_string(env, path, strlen(path)) : em_nil;
}
