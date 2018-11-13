#include "egit.h"

#ifndef EGIT_REMOTE_H
#define EGIT_REMOTE_H

EGIT_DEFUN(remote_lookup, emacs_value _repo, emacs_value _name);

EGIT_DEFUN(remote_autotag, emacs_value _remote);
EGIT_DEFUN(remote_get_refspecs, emacs_value _remote, emacs_value dir);
EGIT_DEFUN(remote_name, emacs_value _remote);
EGIT_DEFUN(remote_owner, emacs_value _remote);
EGIT_DEFUN(remote_pushurl, emacs_value _remote);
EGIT_DEFUN(remote_refspec_count, emacs_value _remote);
EGIT_DEFUN(remote_url, emacs_value _remote);

EGIT_DEFUN(remote_valid_name_p, emacs_value _name);

#endif /* EGIT_REMOTE_H */