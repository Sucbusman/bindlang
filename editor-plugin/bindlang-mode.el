(defvar bindlang-mode-hook nil)

(defun bindlang-electric-proc ()
  "insert procedure definition quickly"
  (interactive)
  (insert "\\->")
  (backward-char 2))

(defvar bindlang-mode-map
  (let ((map (make-keymap)))
    (define-key map "\C-l" 'bindlang-electric-proc)
		map)
  "Keymap for bindlang major mode")

(add-to-list 'auto-mode-alist
             '("\\.bd\\'" . bindlang-mode))

(defvar bindlang-highlights
  `((,(rx (or symbol-start "->")
         (group  (or "eq"  "gt" "lt"
                     "take" "len" "++"
                     "sys/read" "sys/write" "sys/open" "sys/close"
                     "int>str" "str>ints" "int>file"
                     "not" "and" "or"
                     "+" "-" "*" "/"
                     "=>" "'" "#t" "#f" "null"
                     "hd" "tl" "cons" "empty?"
                     "let" "if" "while"
                     "print" ))
         symbol-end)
     (1 font-lock-keyword-face))
    (,(rx  "->") . font-lock-builtin-face)
    (,(rx symbol-start (group (1+ (or word ?_ ?? ?! ?-)))
          (0+ space) "=" (0+ (or space eol)) (group ?\\))
     (1 font-lock-function-name-face) (2 font-lock-builtin-face))
    (,(rx symbol-start (group (1+ (or word ?_ ?? ?! ?-)))
          (0+ space) "=")
     (1 font-lock-variable-name-face))
     (,(rx symbol-start (group ?<) (*? anything) (group ?>))
      (1 font-lock-type-face) (2 font-lock-type-face))
    ))

(defun find-partial-paren ()
  "Find unclosing left parenthesis in a line"
  (save-excursion 
    (end-of-line)
    (let (pos (end (point)) (left 0) (poses []))
      (beginning-of-line)
      (setq pos (point))
      (while (<= pos end)
        (pcase (char-after pos)
          (?\( (progn (setq left (1+ left))
                      (setq poses (vconcat poses
                                           (make-vector 1 pos)))))
          (?\) (setq left (1- left))))
        (setq pos (1+ pos)))
      (cond ((> left 0) (cons left poses))
            ((< left 0) (cons left nil))
            ((cons left nil))))))

(defun bindlang-calculate-indent ()
  "calculate indentation"
  (beginning-of-line)
  (if (bobp)
      0
    (save-excursion
      (forward-line -1)
      (let ((start (point)) (ans 0) (pair (find-partial-paren)) left)
        (setq left (car pair))
        (cond ((> left 0) (1+ (- (aref (cdr pair) (1- left)) start)))
              ((= left 0) (current-indentation))
              (t (progn
                   (while (progn
                            (forward-line -1)
                            (if (bobp) 0
                              (let ((start2 (point)) (pair2 (find-partial-paren)) left2)
                                (setq left2 (car pair2))
                                (setq left (+ left2 left))
                                (cond ((> left 0) (setq ans (1+ (- (aref (cdr pair2) (1- left)) start2))))
                                      ((= left 0) (setq ans (current-indentation))))))
                            ;;do while
                            (< left 0)))
                   ans)))))))

(defun bindlang-indent-line ()
  "Indent current line as bindlang code"
  (interactive)
  (indent-line-to (bindlang-calculate-indent)))

(defvar bindlang-mode-syntax-table
  (let ((st (make-syntax-table)))
    (modify-syntax-entry ?_ "w" st)
    (modify-syntax-entry ?# "w" st)
    (modify-syntax-entry ?- "_" st)
    (modify-syntax-entry ?> "_" st)
    (modify-syntax-entry ?= "_" st)
    (modify-syntax-entry ?\; "<" st)
    (modify-syntax-entry ?\n ">b" st)
    st)
  "Sysntax table for bindlang-mode")

;; (defun bindlang-mode ()
;;   "Major mode for eding bindlng"
;;   (interactive)
;;   (kill-all-local-variables)
;;   (set-syntax-table bindlang-mode-syntax-table)
;;   (use-local-map bindlang-mode-map)
;;   (set (make-local-variable 'font-lock-defaults)
;;        '(bindlang-highlights))
;;   (set (make-local-variable 'indent-line-function)
;;        'bindlang-indent-line)
;;   (setq major-mode 'bindlang-mode)
;;   (setq mode-name "bindlang-mode")
;;   (run-hooks 'bindlang-mode-hook))
(define-derived-mode bindlang-mode fundamental-mode "bindlang"
  "Major mode for bindlang language files."
  (set (make-local-variable 'font-lock-defaults) '(bindlang-highlights))
  (set (make-local-variable 'indent-line-function) 'bindlang-indent-line))
(provide 'bindlang-mode)
