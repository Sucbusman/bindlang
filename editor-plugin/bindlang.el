(defvar bindlang-mode-hook nil)
(defvar bindlang-mode-map
  (let ((map (make-keymap)))
    (define-key map "\C-j" 'newline-and-indent)
		map)
  "Keymap for bindlang major mode")

(add-to-list 'auto-mode-alist
             '("\\.bd\\'" . bindlang-mode))


;;(regexp-opt '("->" "+" "-" "*" "/" "=>" "take" "len" "set"
;;             "=" "\\" "eq" "lt" "gt" "ge" "le" "print") t)

;;(regexp-opt '("<as>") t)
;;(rx (and "<" (*? anything) ">"))

(defconst bindlang-highlights
  (list
   '("\\(->\\|=>\\|eq\\|g[et]\\|l\\(?:en\\|[et]\\)\\|print\\|set\\|take\\|[*+/=\\-]\\)". font-lock-builtin-face)
   '("\\('\\w*'\\)" . font-lock-variable-name-face)
   '("<\\(?:.\\|\\)*?>" . font-lock-type-face))
  "Minimal highlighting expressions for bindlang mode")

(defun bindlang-indent-line ()
  "Indent current line as bindlang code"
  (interactive)
  (beginning-of-line)
  (if (bobp)
      (indent-line-to 0)
    2))

(defvar bindlang-mode-syntax-table
  (let ((st (make-syntax-table)))
    ;;(modify-syntax-entry ?_ "w" st)
    (modify-syntax-entry ?# "<" st)
    (modify-syntax-entry ?\n ">b" st)
    st)
  "Sysntax table for bindlang-mode")

(defun bindlang-mode ()
  "Major mode for eding bindlng"
  (interactive)
  (kill-all-local-variables)
  (set-syntax-table bindlang-mode-syntax-table)
  (use-local-map bindlang-mode-map)
  (set (make-local-variable 'font-lock-defaults)
       '(bindlang-highlights))
  (set (make-local-variable 'indent-line-function)
       'bindlang-indent-line)
  (setq major-mode 'bindlang-mode)
  (setq mode-name "bindlang-mode")
  (run-hooks 'bindlang-mode-hook))

(provide 'bindlang-mode)
