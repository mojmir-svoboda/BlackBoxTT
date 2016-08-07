;;; lint for s7 scheme
;;;
;;; (lint "file.scm") checks file.scm for infelicities
;;; to control the kinds of checks, set the variables below.
;;; for tests and examples, see lint-test in s7test.scm 

(provide 'lint.scm)

(define *report-unused-parameters* #f)                    ; many of these are reported anyway if they are passed some non-#f value
(define *report-unused-top-level-functions* #f)           ; very common in Scheme, but #t makes the ghastly leakage of names obvious
(define *report-shadowed-variables* #f)                   ; shadowed parameters, etc
(define *report-undefined-identifiers* #f)                ; names we can't account for
(define *report-multiply-defined-top-level-functions* #f) ; top-level funcs defined in more than one file
(define *report-nested-if* 4)                             ; 3 is lowest, this sets the nesting level that triggers an if->cond suggestion
(define *report-short-branch* 12)                         ; controls when a lop-sided if triggers a reordering suggestion
(define *report-one-armed-if* 90)                         ; if -> when/unless, can be #f/#t; if an integer, sets tree length which triggers revision (80 is too small)
(define *report-loaded-files* #f)                         ; if load is encountered, include that file in the lint process
(define *report-any-!-as-setter* #t)                      ; unknown funcs/macros ending in ! are treated as setters
(define *report-function-stuff* #t)                       ; checks for missed function uses etc
(define *report-doc-strings* #f)                          ; old-style (CL) doc strings
(define *report-func-as-arg-arity-mismatch* #f)           ; as it says... (slow, and this error almost never happens)
(define *report-constant-expressions-in-do* #f)           ; kinda dumb
(define *report-bad-variable-names* '(l ll O ~))          ; bad names -- a list to check such as:
;;;             '(l ll .. ~ data datum new item info temp tmp temporary val vals value foo bar baz aux dummy O var res retval result count str)
(define *report-built-in-functions-used-as-variables* #f) ; string and length are the most common cases
(define *report-forward-functions* #f)                    ; functions used before being defined
(define *report-sloppy-assoc* #t)                         ; i.e. (cdr (assoc x y)) and the like
(define *report-bloated-arg* 24)                          ; min arg expr tree size that can trigger a rewrite-as-let suggestion

(define *lint* #f)                                        ; the lint let
;; this gives other programs a way to extend or edit lint's tables: for example, the
;;   table of functions that are simple (no side effects) is (*lint* 'no-side-effect-functions)
;;   see snd-lint.scm.


;;; --------------------------------------------------------------------------------

(when (provided? 'pure-s7)
  (define (make-polar mag ang) (complex (* mag (cos ang)) (* mag (sin ang))))
  
  (define (char-ci=? . chars) (apply char=? (map char-upcase chars)))
  (define (char-ci<=? . chars) (apply char<=? (map char-upcase chars)))
  (define (char-ci>=? . chars) (apply char>=? (map char-upcase chars)))
  (define (char-ci<? . chars) (apply char<? (map char-upcase chars)))
  (define (char-ci>? . chars) (apply char>? (map char-upcase chars)))
  
  (define (string-ci=? . strs) (apply string=? (map string-upcase strs)))
  (define (string-ci<=? . strs) (apply string<=? (map string-upcase strs)))
  (define (string-ci>=? . strs) (apply string>=? (map string-upcase strs)))
  (define (string-ci<? . strs) (apply string<? (map string-upcase strs)))
  (define (string-ci>? . strs) (apply string>? (map string-upcase strs)))
  
  (define (let->list e)
    (if (let? e)
	(reverse! (map values e))
	(error 'wrong-type-arg "let->list argument should be an environment: ~A" str))))


(format *stderr* "loading lint.scm~%")

(set! reader-cond #f)
(define-macro (reader-cond . clauses) `(values))          ; clobber reader-cond to avoid (incorrect) unbound-variable errors

#|
;; debugging version
(define-expansion (lint-format str caller . args)
  `(begin
     (format outport "lint.scm line ~A~%" ,(port-line-number))
     (lint-format-1 ,str ,caller ,@args)))

(define-expansion (lint-format* caller . args)
  `(begin
     (format outport "lint.scm line ~A~%" ,(port-line-number))
     (lint-format*-1 ,caller ,@args)))
|#

(define-macro (let*-temporarily vars . body)
  `(with-let (#_inlet :orig (#_curlet) 
		      :saved (#_list ,@(map car vars)))
     (dynamic-wind
	 (lambda () #f)
	 (lambda ()
	   (with-let orig
	     ,@(map (lambda (v)
		      `(set! ,(car v) ,(cadr v)))
		    vars)
	     ,@body))
	 (lambda ()
	   ,@(map (let ((ctr -1))
		    (lambda (v)
		      (if (symbol? (car v))
			  `(set! (orig ',(car v)) (list-ref saved ,(set! ctr (+ ctr 1))))
			  `(set! (with-let orig ,(car v)) (list-ref saved ,(set! ctr (+ ctr 1)))))))
		  vars)))))

(define-macro (let-temporarily vars . body)
  `(with-let (#_inlet :orig (#_curlet) 
		      :saved (#_list ,@(map car vars))
		      :new (#_list ,@(map cadr vars)))
     (dynamic-wind
	 (lambda () #f)
	 (lambda () ; this could be (with-let orig (let ,vars ,@body)) but I want to handle stuff like individual vector locations
	   ,@(map (let ((ctr -1))
		    (lambda (v)
		      (if (symbol? (car v))
			  `(set! (orig ',(car v)) (list-ref new ,(set! ctr (+ ctr 1))))
			  `(set! (with-let orig ,(car v)) (list-ref new ,(set! ctr (+ ctr 1)))))))
		  vars)
	   (with-let orig ,@body)) 
	 (lambda ()
	   ,@(map (let ((ctr -1))
		    (lambda (v)
		      (if (symbol? (car v))
			  `(set! (orig ',(car v)) (list-ref saved ,(set! ctr (+ ctr 1))))
			  `(set! (with-let orig ,(car v)) (list-ref saved ,(set! ctr (+ ctr 1)))))))
		  vars)))))


;;; --------------------------------------------------------------------------------
(define lint

  (let ((no-side-effect-functions 
	 (let ((ht (make-hash-table)))
	   (for-each
	    (lambda (op) 
	      (hash-table-set! ht op #t))
	    '(* + - / < <= = > >= 
	      abs acos acosh and angle append aritable? arity ash asin asinh assoc assq assv atan atanh 
	      begin boolean? byte-vector byte-vector?
	      caaaar caaadr caaar caadar caaddr caadr caar cadaar cadadr cadar caddar cadddr caddr cadr
	      call-with-input-string call-with-input-file
	      c-pointer c-pointer? c-object? call-with-exit car case catch cdaaar cdaadr cdaar cdadar cdaddr cdadr cdar cddaar cddadr
	      cddar cdddar cddddr cdddr cddr cdr ceiling char->integer char-alphabetic? char-ci<=?
	      char-ci<? char-ci=? char-ci>=? char-ci>? char-downcase char-lower-case? char-numeric? 
	      char-position char-ready? char-upcase char-upper-case? char-whitespace? char<=? char<?
	      char=? char>=? char>? char? complex complex? cond cons constant? continuation? cos
	      cosh curlet current-error-port current-input-port current-output-port cyclic-sequences
	      defined? denominator dilambda? do dynamic-wind
	      eof-object? eq? equal? eqv? even? exact->inexact exact? exp expt
	      float? float-vector float-vector-ref float-vector? floor for-each funclet 
	      gcd gensym gensym? ; why was gensym omitted earlier?
	      hash-table hash-table* hash-table-entries hash-table-ref hash-table? help hook-functions
	      if imag-part inexact->exact inexact? infinite? inlet input-port? 
	      int-vector int-vector-ref int-vector? iterator-at-end? iterator-sequence integer->char
	      integer-decode-float integer-length integer? iterator?
	      keyword->symbol keyword?
	      lambda lambda* lcm let->list length let let* let-ref let? letrec letrec* list list->string list->vector list-ref
	      list-tail list? log logand logbit? logior lognot logxor
	      macro? magnitude make-byte-vector make-float-vector make-int-vector make-hash-table make-hook make-iterator make-keyword make-list make-polar
	      make-rectangular make-shared-vector make-string make-vector map max member memq memv min modulo morally-equal?
	      nan? negative? not null? number->string number? numerator
	      object->string odd? openlet? or outlet output-port? owlet
	      pair-line-number pair-filename pair? port-closed? port-filename port-line-number positive? procedure-documentation
	      procedure-setter procedure-signature procedure-source procedure? proper-list? provided?
	      quasiquote quote quotient
	      random-state random-state->list random-state? rational? rationalize real-part real? remainder reverse rootlet round
	      s7-version sequence? sin sinh square sqrt stacktrace string string->list string->number string->symbol string-append 
	      string-ci<=? string-ci<? string-ci=? string-ci>=? string-ci>? string-downcase string-length
	      string-position string-ref string-upcase string<=? string<? string=? string>=? string>? string?
	      sublet substring symbol symbol->dynamic-value symbol->keyword symbol->string symbol->value symbol?
	      tan tanh tree-leaves truncate
	      unless
	      values vector vector-append vector->list vector-dimensions vector-length vector-ref vector?
	      when with-baffle with-let with-input-from-file with-input-from-string with-output-to-string
	      zero?
	      #_{list} #_{apply_values} #_{append} unquote))
	   ;; do not include file-exists? or directory?
	   ;; should this include peek-char or unlet ?
	   ht))

	(built-in-functions (let ((ht (make-hash-table)))
			      (for-each
			       (lambda (op) 
				 (hash-table-set! ht op #t))
			       '(symbol? gensym? keyword? let? openlet? iterator? constant? macro? c-pointer? c-object? 
			         input-port? output-port? eof-object? integer? number? real? complex? rational? random-state? 
			         char? string? list? pair? vector? float-vector? int-vector? byte-vector? hash-table? 
			         continuation? procedure? dilambda? boolean? float? proper-list? sequence? null? gensym 
			         symbol->string string->symbol symbol symbol->value symbol->dynamic-value symbol-access 
			         make-keyword symbol->keyword keyword->symbol outlet rootlet curlet unlet sublet varlet 
			         cutlet inlet owlet coverlet openlet let-ref let-set! make-iterator iterate iterator-sequence
			         iterator-at-end? provided? provide defined? c-pointer port-line-number port-filename 
			         pair-line-number pair-filename port-closed? current-input-port current-output-port 
			         current-error-port let->list char-ready? close-input-port close-output-port flush-output-port 
			         open-input-file open-output-file open-input-string open-output-string get-output-string 
			         newline write display read-char peek-char write-char write-string read-byte write-byte 
			         read-line read-string read call-with-input-string call-with-input-file with-input-from-string 
			         with-input-from-file call-with-output-string call-with-output-file with-output-to-string 
			         with-output-to-file real-part imag-part numerator denominator even? odd? zero? positive? 
			         negative? infinite? nan? complex magnitude angle rationalize abs exp log sin cos tan asin 
			         acos atan sinh cosh tanh asinh acosh atanh sqrt expt floor ceiling truncate round lcm gcd
			         + - * / max min quotient remainder modulo = < > <= >= logior logxor logand lognot ash 
			         random-state random inexact->exact exact->inexact integer-length make-polar make-rectangular 
			         logbit? integer-decode-float exact? inexact? random-state->list number->string string->number 
			         char-upcase char-downcase char->integer integer->char char-upper-case? char-lower-case? 
			         char-alphabetic? char-numeric? char-whitespace? char=? char<? char>? char<=? char>=? 
			         char-position string-position make-string string-ref string-set! string=? string<? string>? 
			         string<=? string>=? char-ci=? char-ci<? char-ci>? char-ci<=? char-ci>=? string-ci=? string-ci<? 
			         string-ci>? string-ci<=? string-ci>=? string-copy string-fill! list->string string-length 
			         string->list string-downcase string-upcase string-append substring string object->string 
			         format cons car cdr set-car! set-cdr! caar cadr cdar cddr caaar caadr cadar cdaar caddr 
			         cdddr cdadr cddar caaaar caaadr caadar cadaar caaddr cadddr cadadr caddar cdaaar cdaadr 
			         cdadar cddaar cdaddr cddddr cddadr cdddar assoc member list list-ref list-set! list-tail 
			         make-list length copy fill! reverse reverse! sort! append assq assv memq memv vector-append 
			         list->vector vector-fill! vector-length vector->list vector-ref vector-set! vector-dimensions 
			         make-vector make-shared-vector vector float-vector make-float-vector float-vector-set! 
			         float-vector-ref int-vector make-int-vector int-vector-set! int-vector-ref ->byte-vector 
			         byte-vector make-byte-vector hash-table hash-table* make-hash-table hash-table-ref 
			         hash-table-set! hash-table-entries cyclic-sequences call/cc call-with-current-continuation 
			         call-with-exit load autoload eval eval-string apply for-each map dynamic-wind values 
			         catch throw error procedure-documentation procedure-signature help procedure-source funclet 
			         procedure-setter arity aritable? not eq? eqv? equal? morally-equal? gc s7-version emergency-exit 
			         exit dilambda make-hook hook-functions stacktrace tree-leaves
				 #_{list} #_{apply_values} #_{append} unquote))
			      ht))

	(makers (let ((h (make-hash-table)))
		  (for-each
		   (lambda (op)
		     (set! (h op) #t))
		   '(gensym sublet inlet make-iterator let->list random-state random-state->list number->string
		     make-string string string-copy copy list->string string->list string-append substring object->string
		     format cons list make-list reverse append vector-append list->vector vector->list make-vector
		     make-shared-vector vector make-float-vector float-vector make-int-vector int-vector byte-vector
		     hash-table hash-table* make-hash-table make-hook #_{list} #_{append} gentemp)) ; gentemp for other schemes
		  h))

	(non-negative-ops (let ((h (make-hash-table)))
			    (for-each
			     (lambda (op)
			       (set! (h op) #t))
			     '(string-length vector-length abs magnitude denominator gcd lcm tree-leaves
			       char->integer byte-vector-ref byte-vector-set! hash-table-entries write-byte
			       char-position string-position pair-line-number port-line-number))
			    h))
	
	(numeric-ops (let ((h (make-hash-table)))
		       (for-each
			(lambda (op)
			  (set! (h op) #t))
			'(+ * - / 
			  sin cos tan asin acos atan sinh cosh tanh asinh acosh atanh 
			  log exp expt sqrt make-polar complex
			  imag-part real-part abs magnitude angle max min exact->inexact
			  modulo remainder quotient lcm gcd
			  rationalize inexact->exact random
			  logior lognot logxor logand numerator denominator 
			  floor round truncate ceiling ash))
		       h))

	(bools (let ((h (make-hash-table)))
		 (for-each
		  (lambda (op)
		    (set! (h op) #t))
		  '(symbol? integer? rational? real? number? complex? float? keyword? gensym? byte-vector? string? list? sequence?
		    char? boolean? float-vector? int-vector? vector? let? hash-table? input-port? null? pair? proper-list?
		    output-port? iterator? continuation? dilambda? procedure? macro? random-state? eof-object? c-pointer?
		    unspecified? c-object? constant?))
		 h))

	(booleans (let ((h (make-hash-table)))
		  (for-each
		   (lambda (op)
		     (set! (h op) #t))
		   '(symbol? integer? rational? real? number? complex? float? keyword? gensym? byte-vector? string? list? sequence?
		     char? boolean? float-vector? int-vector? vector? let? hash-table? input-port? null? pair? proper-list?
		     output-port? iterator? continuation? dilambda? procedure? macro? random-state? eof-object? c-pointer? c-object?
		     unspecified? exact? inexact? defined? provided? even? odd? char-whitespace? char-numeric? char-alphabetic?
		     negative? positive? zero? constant? infinite? nan? char-upper-case? char-lower-case? directory? file-exists?))
		  h))

	(reversibles (let ((h (make-hash-table)))
		       (for-each
			(lambda (op)
			  (set! (h (car op)) (cadr op)))
			'((< >) (> <) (<= >=) (>= <=)
			  (* *) (+ +) (= =) (char=? char=?) (string=? string=?)
			  (eq? eq?) (eqv? eqv?) (equal? equal?) (morally-equal? morally-equal?)
			  (logand logand) (logxor logxor) (logior logior)
			  (max max) (min min) (lcm lcm) (gcd gcd)
			  (char<? char>?) (char>? char<?) (char<=? char>=?) (char>=? char<=?) 
			  (string<? string>?) (string>? string<?) (string<=? string>=?) (string>=? string<=?) 
			  (char-ci<? char-ci>?) (char-ci>? char-ci<?) (char-ci<=? char-ci>=?) (char-ci>=? char-ci<=?)
			  (string-ci<? string-ci>?) (string-ci>? string-ci<?) (string-ci<=? string-ci>=?) (string-ci>=? string-ci<=?)))
		       h))
    
	(syntaces (let ((h (make-hash-table)))
		    (for-each
		     (lambda (op)
		       (set! (h op) #t))
		     '(quote if begin let let* letrec letrec* cond case or and do set! unless when
		       with-let with-baffle
		       lambda lambda* define define* 
		       define-macro define-macro* define-bacro define-bacro* 
		       define-constant define-expansion))
		    h))

	(outport #t)
	(linted-files ())
	(big-constants (make-hash-table))
	(equable-closures (make-hash-table))
	(other-names-counts (make-hash-table))
	(*e* #f)
	(other-identifiers (make-hash-table))
	(quote-warnings 0)
	(last-simplify-boolean-line-number -1)
	(last-simplify-numeric-line-number -1)
	(last-simplify-cxr-line-number -1)
	(last-if-line-number -1)
	(last-checker-line-number -1)
	(last-cons-line-number -1)
	(last-rewritten-internal-define #f)
	(line-number -1)
	(pp-left-margin 4)
	(lint-left-margin 1)
	(*current-file* "")
	(*top-level-objects* (make-hash-table))
	(*output-port* *stderr*)
	(*max-cdr-len* 16)) ; 40 is too high, 24 questionable, if #f the let+do rewrite is turned off
    
    (set! *e* (curlet))
    (set! *lint* *e*)                ; external access to (for example) the built-in-functions hash-table via (*lint* 'built-in-functions)


    ;; -------- lint-format --------
    (define target-line-length 80)

    (define (truncated-list->string form)
      ;; return form -> string with limits on its length
      (let* ((str (object->string form))
	     (len (length str)))
	(if (< len target-line-length)
	    str
	    (do ((i (- target-line-length 6) (- i 1)))
		((or (= i 40)
		     (char-whitespace? (str i)))
		 (string-append (substring str 0 (if (<= i 40) 
						     (- target-line-length 6)
						     i))
				"..."))))))
    
    (define lint-pp #f) ; avoid crosstalk with other schemes' definitions of pp and pretty-print (make-var also collides)
    (define lint-pretty-print #f)
    (let ()
      (require write.scm)
      (set! lint-pp pp);
      (set! lint-pretty-print pretty-print))
    
    (define (lists->string f1 f2)
      ;; same but 2 strings that may need to be lined up vertically
      (let ((str1 (object->string f1))
	    (str2 (object->string f2)))
	(let ((len1 (length str1))
	      (len2 (length str2)))
	  (when (> len1 target-line-length)
	    (set! str1 (truncated-list->string f1))
	    (set! len1 (length str1)))
	  (when (> len2 target-line-length)
	    (set! ((funclet lint-pretty-print) '*pretty-print-left-margin*) pp-left-margin)
	    (set! ((funclet lint-pretty-print) '*pretty-print-length*) (- 114 pp-left-margin))
	    (set! str2 (lint-pp f2))
	    (set! len2 (length str2)))
	  (format #f (if (< (+ len1 len2) target-line-length)
			 (values "~A -> ~A" str1 str2)
			 (values "~%~NC~A ->~%~NC~A" pp-left-margin #\space str1 pp-left-margin #\space str2))))))
    
    (define (truncated-lists->string f1 f2)
      ;; same but 2 strings that may need to be lined up vertically and both are truncated
      (let ((str1 (object->string f1))
	    (str2 (object->string f2)))
	(let ((len1 (length str1))
	      (len2 (length str2)))
	  (when (> len1 target-line-length)
	    (set! str1 (truncated-list->string f1))
	    (set! len1 (length str1)))
	  (when (> len2 target-line-length)
	    (set! str2 (truncated-list->string f2))
	    (set! len2 (length str2)))
	  (format #f (if (< (+ len1 len2) target-line-length)
			 (values "~A -> ~A" str1 str2)
			 (values "~%~NC~A ->~%~NC~A" pp-left-margin #\space str1 pp-left-margin #\space str2))))))
    
    (define made-suggestion 0)

    (define (lint-format str caller . args)
      (let ((outstr (apply format #f 
			   (string-append (if (< 0 line-number 100000)
					      "~NC~A (line ~D): " 
					      "~NC~A: ")
					  str "~%")
			   lint-left-margin #\space
			   (truncated-list->string caller)
			   (if (< 0 line-number 100000)
			       (values line-number args)
			       args))))
	(set! made-suggestion (+ made-suggestion 1))
	(display outstr outport)
	(if (> (length outstr) (+ target-line-length 40))
	    (newline outport))))

    (define (lint-format* caller . strs)
      (let* ((outstr (format #f 
			   (if (< 0 line-number 100000)
			       "~NC~A (line ~D): " 
			       "~NC~A:~A")
			   lint-left-margin #\space
			   (truncated-list->string caller)
			   (if (< 0 line-number 100000)
			       line-number
			       " ")))
	     (current-end (length outstr)))
	;; (set! made-suggestion (+ made-suggestion 1))
	(display outstr outport)
	(for-each (lambda (s)
		    (let ((len (length s)))
		      (if (> (+ len current-end) target-line-length)
			  (begin
			    (format outport "~%~NC~A" (+ lint-left-margin 4) #\space s)
			    (set! current-end len))
			  (begin
			    (display s outport)
			    (set! current-end (+ current-end len))))))
		  strs)
	(newline outport)))

    (define (local-line-number tree)
      (let ((tree-line (if (pair? tree) (pair-line-number tree) 0)))
	(if (and (< 0 tree-line 100000)
		 (not (= tree-line line-number)))
	    (format #f " (line ~D)" tree-line)
	    "")))

    
    ;; -------- vars -------- 
    (define var-name car)
    (define (var? v) (and (pair? v) (let? (cdr v))))
    (define var-member assq)
    
    (define var-ref     (dilambda (lambda (v) (let-ref (cdr v) 'ref))     (lambda (v x) (let-set! (cdr v) 'ref x))))
    (define var-set     (dilambda (lambda (v) (let-ref (cdr v) 'set))     (lambda (v x) (let-set! (cdr v) 'set x))))
    (define var-history (dilambda (lambda (v) (let-ref (cdr v) 'history)) (lambda (v x) (let-set! (cdr v) 'history x))))
    (define var-ftype   (dilambda (lambda (v) (let-ref (cdr v) 'ftype))   (lambda (v x) (let-set! (cdr v) 'ftype x))))
    (define var-arglist (dilambda (lambda (v) (let-ref (cdr v) 'arglist)) (lambda (v x) (let-set! (cdr v) 'arglist x))))
    (define var-definer (dilambda (lambda (v) (let-ref (cdr v) 'definer)) (lambda (v x) (let-set! (cdr v) 'definer x))))
    (define var-leaves  (dilambda (lambda (v) (let-ref (cdr v) 'leaves))  (lambda (v x) (let-set! (cdr v) 'leaves x))))
    (define var-scope   (dilambda (lambda (v) (let-ref (cdr v) 'scope))   (lambda (v x) (let-set! (cdr v) 'scope x))))
    (define var-setters (dilambda (lambda (v) (let-ref (cdr v) 'setters)) (lambda (v x) (let-set! (cdr v) 'setters x))))
    (define var-env     (dilambda (lambda (v) (let-ref (cdr v) 'env))     (lambda (v x) (let-set! (cdr v) 'env x))))
    (define var-decl    (dilambda (lambda (v) (let-ref (cdr v) 'decl))    (lambda (v x) (let-set! (cdr v) 'decl x))))
    (define var-match-list (dilambda (lambda (v) (let-ref (cdr v) 'match-list)) (lambda (v x) (let-set! (cdr v) 'match-list x))))
    (define var-initial-value (lambda (v) (let-ref (cdr v) 'initial-value))) ; not settable

    (define var-side-effect (dilambda (lambda (v) 
					(if (null? (let-ref (cdr v) 'side-effect))
					    (let-set! (cdr v) 'side-effect (get-side-effect v))
					    (let-ref (cdr v) 'side-effect)))
				      (lambda (v x) 
					(let-set! (cdr v) 'side-effect x))))

    (define var-signature (dilambda (lambda (v) 
				      (if (null? (let-ref (cdr v) 'signature))
					  (let-set! (cdr v) 'signature (get-signature v))
					  (let-ref (cdr v) 'signature)))
				    (lambda (v x) 
				      (let-set! (cdr v) 'signature x))))
    
    (define* (make-var name initial-value definer)
      (let ((old (hash-table-ref other-identifiers name)))
	(cons name (inlet 'initial-value initial-value 
			  'definer definer 
			  'history (if old 
				       (begin
					 (hash-table-set! other-identifiers name #f)
					 (if initial-value (cons initial-value old) old))
				       (if initial-value (list initial-value) ()))
			  'scope ()
			  'setters ()
			  'set 0 
			  'ref (if old (length old) 0)))))
    
    
    ;; -------- the usual list functions --------
    (define (remove item sequence)
      (cond ((null? sequence) ())
	    ((equal? item (car sequence)) (cdr sequence))
	    (else (cons (car sequence) (remove item (cdr sequence))))))
    
    (define (remove-all item sequence)
      (map (lambda (x)
	     (if (equal? x item)
		 (values)
		 x))
	   sequence))
    
    (define (remove-if p lst)
      (cond ((null? lst) ())
	    ((p (car lst)) (remove-if p (cdr lst)))
	    (else (cons (car lst)
			(remove-if p (cdr lst))))))
    
    (define (lint-remove-duplicates lst env)
      (reverse (let rem-dup ((lst lst)
			     (nlst ()))
		 (cond ((null? lst) nlst)
		       ((and (member (car lst) nlst)
			     (not (and (pair? (car lst))
				       (side-effect? (car lst) env))))
			(rem-dup (cdr lst) nlst))
		       (else (rem-dup (cdr lst) (cons (car lst) nlst)))))))
    
    (define applicable? arity)
    
    (define every? 
      (let ((documentation "(every? func sequence) returns #t if func approves of every member of sequence")
	    (signature '(boolean? procedure? sequence?)))
	(lambda (f sequence)
	  (call-with-exit
	   (lambda (return) 
	     (for-each (lambda (arg) (if (not (f arg)) (return #f))) sequence)
	     #t)))))
    
    (define any? 
      (let ((documentation "(any? func sequence) returns #t if func approves of any member of sequence")
	    (signature '(boolean? procedure? sequence?)))
	(lambda (f sequence)
	  (call-with-exit
	   (lambda (return) 
	     (for-each (lambda (arg) (if (f arg) (return #t))) sequence)
	     #f)))))
    
    (define collect-if 
      (let ((documentation "(collect-if type func sequence) gathers the elements of sequence that satisfy func, and returns them via type:\n\
              (collect-if list integer? #(1.4 2/3 1 1+i 2)) -> '(1 2)"))
	(lambda (type f sequence)
	  (apply type (map (lambda (arg) (if (f arg) arg (values))) sequence)))))
    
    (define find-if 
      (let ((documentation "(find-if func sequence) applies func to each member of sequence.\n\
              If func approves of one, find-if returns that member of the sequence"))
	(lambda (f sequence)
	  (call-with-exit
	   (lambda (return)
	     (for-each (lambda (arg)
			 (if (f arg)
			     (return arg)))
		       sequence)
	     #f)))))
    
    
    ;; -------- trees --------
    (define copy-tree 
      (let ((documentation "(copy-tree lst) returns a full copy of lst"))
	(lambda (lis)
	  (if (pair? lis)
	      (cons (copy-tree (car lis))
		    (copy-tree (cdr lis)))
	      lis))))
    
    (define (tree-count1 x tree count)
      (if (eq? x tree)
	  (+ count 1)
	  (if (or (>= count 2)
		  (not (pair? tree))
		  (eq? (car tree) 'quote))
	      count
	      (tree-count1 x (car tree) (tree-count1 x (cdr tree) count)))))

    (define (tree-count2 x tree count)
      (if (eq? x tree)
	  (+ count 1)
	  (if (or (>= count 3)
		  (not (pair? tree))
		  (eq? (car tree) 'quote))
	      count
	      (tree-count2 x (car tree) (tree-count2 x (cdr tree) count)))))
    
    (define (proper-tree? tree)
      (or (not (pair? tree))
	  (and (proper-list? tree)
	       (every? proper-tree? (cdr tree)))))

    (define (gather-symbols tree)
      (let ((syms ()))
	(let walk ((p tree))
	  (if (pair? p)
	      (if (symbol? (car p))
		  (if (not (eq? (car p) 'quote))
		      (for-each (lambda (a)
				  (if (symbol? a)
				      (if (not (memq a syms))
					  (set! syms (cons a syms)))
				      (if (pair? a) (walk a))))
				(cdr p)))
		  (if (pair? (car p))
		      (begin
			(walk (car p))
			(walk (cdr p)))))
	      (if (and (symbol? tree)
		       (not (memq tree syms)))
		  (set! syms (cons tree syms)))))
	syms))
    
    (define (tree-arg-member sym tree)
      (and (proper-list? tree)
	   (or (and (memq sym (cdr tree))
		    tree)
	       (and (pair? (car tree))
		    (tree-arg-member sym (car tree)))
	       (and (pair? (cdr tree))
		    (call-with-exit
		     (lambda (return)
		       (for-each
			(lambda (b)
			  (cond ((and (pair? b)
				      (tree-arg-member sym b))
				 => return)))
			(cdr tree))
		       #f))))))
    
    (define (tree-memq sym tree)     ; ignore quoted lists, accept symbol outside a pair
      (or (eq? sym tree)
	  (and (pair? tree)
	       (not (eq? (car tree) 'quote))
	       (or (eq? (car tree) sym)
		   (tree-memq sym (car tree))
		   (tree-memq sym (cdr tree))))))

    (define (tree-member sym tree)
      (and (pair? tree)
	   (or (eq? (car tree) sym)
	       (tree-member sym (car tree))
	       (tree-member sym (cdr tree)))))
    
    (define (tree-unquoted-member sym tree)
      (and (pair? tree)
	   (not (eq? (car tree) 'quote))
	   (or (eq? (car tree) sym)
	       (tree-unquoted-member sym (car tree))
	       (tree-unquoted-member sym (cdr tree)))))
    
    (define (tree-car-member sym tree)
      (and (pair? tree)
	   (or (eq? (car tree) sym)
	       (and (pair? (car tree))
		    (tree-car-member sym (car tree)))
	       (and (pair? (cdr tree))
		    (member sym (cdr tree) tree-car-member)))))
    
    (define (tree-sym-set-member sym set tree) ; sym as arg, set as car
      (and (pair? tree)
	   (or (memq (car tree) set)
	       (and (pair? (car tree))
		    (tree-sym-set-member sym set (car tree)))
	       (and (pair? (cdr tree))
		    (or (member sym (cdr tree))
			(member #f (cdr tree) (lambda (a b) (tree-sym-set-member sym set b))))))))
    
    (define (tree-set-member set tree)
      (and (pair? tree)
	   (not (eq? (car tree) 'quote))
	   (or (memq (car tree) set)
	       (tree-set-member set (car tree))
	       (tree-set-member set (cdr tree)))))
    
    (define (tree-table-member table tree)
      (and (pair? tree)
	   (or (hash-table-ref table (car tree))
	       (tree-table-member table (car tree))
	       (tree-table-member table (cdr tree)))))
    
    (define (tree-set-car-member set tree) ; set as car
      (and (pair? tree)
	   (or (and (memq (car tree) set)
		    tree)
	       (and (pair? (car tree))
		    (tree-set-car-member set (car tree)))
	       (and (pair? (cdr tree))
		    (member #f (cdr tree) (lambda (a b) (tree-set-car-member set b)))))))
    
    (define (tree-table-car-member set tree) ; hash-table as car
      (and (pair? tree)
	   (or (and (hash-table-ref set (car tree))
		    tree)
	       (and (pair? (car tree))
		    (tree-table-car-member set (car tree)))
	       (and (pair? (cdr tree))
		    (member #f (cdr tree) (lambda (a b) (tree-table-car-member set b)))))))
    
    (define (maker? tree)
      (tree-table-car-member makers tree))
    
    (define (tree-symbol-walk tree syms)
      (if (pair? tree)
	  (if (eq? (car tree) 'quote)
	      (if (and (pair? (cdr tree))
		       (symbol? (cadr tree))
		       (not (memq (cadr tree) (car syms))))
		  (tree-symbol-walk (cddr tree) (begin (set-car! syms (cons (cadr tree) (car syms))) syms)))
	      (if (eq? (car tree) {list})
		  (if (and (pair? (cdr tree))
			   (pair? (cadr tree))
			   (eq? (caadr tree) 'quote)
			   (symbol? (cadadr tree))
			   (not (memq (cadadr tree) (cadr syms))))
		      (tree-symbol-walk (cddr tree) (begin (list-set! syms 1 (cons (cadadr tree) (cadr syms))) syms)))
		  (begin
		    (tree-symbol-walk (car tree) syms)
		    (tree-symbol-walk (cdr tree) syms))))))
    
    
    ;; -------- types --------

    (define (quoted-undotted-pair? x)
      (and (pair? x)
	   (eq? (car x) 'quote)
	   (pair? (cdr x))
	   (pair? (cadr x))
	   (positive? (length (cadr x)))))
    
    (define (quoted-null? x)
      (and (pair? x)
	   (eq? (car x) 'quote)
	   (pair? (cdr x))
	   (null? (cadr x))))

    (define (any-null? x)
      (or (null? x)
	  (and (pair? x)
	       (case (car x)
		 ((quote)
		  (and (pair? (cdr x))
		       (null? (cadr x))))
		 ((list)
		  (null? (cdr x)))
		 (else #f)))))
    
    (define (quoted-not? x)
      (and (pair? x)
	   (eq? (car x) 'quote)
	   (pair? (cdr x))
	   (not (cadr x))))
    
    (define (quoted-symbol? x)
      (and (pair? x)
	   (eq? (car x) 'quote)
	   (pair? (cdr x))
	   (symbol? (cadr x))))
    
    (define (code-constant? x)
      (and (or (not (symbol? x))
	       (keyword? x))
	   (or (not (pair? x))
	       (eq? (car x) 'quote))))
    
    (define (just-symbols? form)
      (or (null? form)
	  (symbol? form)
	  (and (pair? form)
	       (symbol? (car form))
	       (just-symbols? (cdr form)))))
    
    (define (list-any? f lst)
      (if (pair? lst)
	  (or (f (car lst))
	      (list-any? f (cdr lst)))
	  (f lst)))
    
    (define syntax? 
      (let ((syns (let ((h (make-hash-table)))
		    (for-each (lambda (x)
				(hash-table-set! h x #t))
			      (list quote if when unless begin set! let let* letrec letrec* cond and or case do
				    lambda lambda* define define* define-macro define-macro* define-bacro define-bacro*
				    define-constant with-baffle macroexpand with-let))
		    h)))
	(lambda (obj) ; a value, not a symbol
	  (hash-table-ref syns obj))))

    
    ;; -------- func info --------
    (define (arg-signature fnc env)
      (and (symbol? fnc)
	   (let ((fd (var-member fnc env)))
	     (if (var? fd)
		 (and (symbol? (var-ftype fd))
		      (var-signature fd))
		 (or (and (eq? *e* *lint*)
			  (procedure-signature fnc))
		     (let ((f (symbol->value fnc *e*)))
		       (and (procedure? f)
			    (procedure-signature f))))))))
    
    (define (arg-arity fnc env)
      (and (symbol? fnc)
	   (let ((fd (var-member fnc env)))
	     (if (var? fd)
		 (and (not (eq? (var-decl fd) 'error))
		      (arity (var-decl fd)))
		 (let ((f (symbol->value fnc *e*)))
		   (and (procedure? f)
			(arity f)))))))
    
    (define (dummy-func caller form f)
      (catch #t 
	(lambda ()
	  (eval f))
	(lambda args
	  (lint-format* caller
			(string-append "in " (truncated-list->string form) ", ")
			(apply format #f (cadr args))))))
    
    (define (count-values body)
      (let ((mn #f)
	    (mx #f))
	(if (pair? body)
	    (let counter ((ignored #f) ; 'ignored is for member's benefit
			  (tree (list-ref body (- (length body) 1))))
	      (if (pair? tree)
		  (if (eq? (car tree) 'values)
		      (let ((args (- (length tree) 1)))
			(for-each (lambda (p)
				    (if (and (pair? p) (eq? (car p) 'values))
					(set! args (- (+ (args (length p)) 2)))))
				  (cdr tree))
			(set! mn (min (or mn args) args))
			(set! mx (max (or mx args) args)))
		      (begin
			(if (pair? (car tree))
			    (counter 'values (car tree)))
			(if (pair? (cdr tree))
			    (member #f (cdr tree) counter)))))
	      #f)) ; return #f so member doesn't quit early
	(and mn (list mn mx))))
    

    (define (get-signature v)

      (define (signer endb env)
	(and (not (side-effect? endb env))
	     (cond ((not (pair? endb))
		    (and (not (symbol? endb))
			 (list (->lint-type endb))))
		   ((arg-signature (car endb) env) 
		    => (lambda (a)
			 (and (pair? a) 
			      (list (car a)))))
		   ((and (eq? (car endb) 'if)
			 (pair? (cddr endb)))
		    (let ((a1 (signer (caddr endb) env))
			  (a2 (and (pair? (cdddr endb))
				   (signer (cadddr endb) env))))
		      (if (not a2)
			  a1
			  (and (equal? a1 a2) a1))))
		   (else #f))))
      
      (let ((ftype (var-ftype v))
	    (initial-value (var-initial-value v))
	    (arglist (var-arglist v))
	    (env (var-env v)))

	(let ((body (and (memq ftype '(define define* lambda lambda* let))
			 (cddr initial-value))))

	  (and (pair? body)
	       (let ((sig (signer (list-ref body (- (length body) 1)) env)))
		 (if (not (pair? sig))
		     (set! sig (list #t)))
		 
		 (when (and (proper-list? arglist)
			    (not (any? keyword? arglist)))
		   (for-each
		    (lambda (arg)       ; new function's parameter
		      (set! sig (cons #t sig))
		      ;; (if (pair? arg) (set! arg (car arg)))
		      ;; causes trouble when tree-count1 sees keyword args in s7test.scm
		      (if (= (tree-count1 arg body 0) 1)
			  (let ((p (tree-arg-member arg body)))
			    (when (pair? p)
			      (let ((f (car p))
				    (m (memq arg (cdr p))))
				(if (pair? m)
				    (let ((fsig (arg-signature f env)))
				      (if (pair? fsig)
					  (let ((chk (catch #t (lambda () (fsig (- (length p) (length m)))) (lambda args #f))))
					    (if (and (symbol? chk) ; it defaults to #t
						     (not (memq chk '(integer:any? integer:real?))))
						(set-car! sig chk)))))))))))
		    arglist))
		 (and (any? (lambda (a) (not (eq? a #t))) sig)
		      (reverse sig)))))))

    (define (args->proper-list args)
      (cond ((symbol? args)	(list args))
	    ((not (pair? args))	args)
	    ((pair? (car args))	(cons (caar args) (args->proper-list (cdr args))))
	    (else               (cons (car args) (args->proper-list (cdr args))))))
    
    (define (out-vars func-name arglist body) ; t367 has tests
      (let ((ref ())
	    (set ()))
	(let var-walk ((tree body)
		       (e (cons func-name arglist)))
	  (define (var-walk-body tree e) 
	    (when (pair? tree)
	      (for-each (lambda (p) (set! e (var-walk p e))) tree)))
	  (define (shadowed v) 
	    (if (and (or (memq v e) (memq v ref))
		     (not (memq v set)))
		(set! set (cons v set)))
	    v)
	  (if (symbol? tree)
	      (if (not (or (memq tree e) (memq tree ref) (defined? tree (rootlet))))
		  (set! ref (cons tree ref)))
	      (when (pair? tree)
		(if (not (pair? (cdr tree)))
		    (var-walk (car tree) e)
		    (case (car tree)
		      ((set! vector-set! list-set! hash-table-set! float-vector-set! int-vector-set! 
			     string-set! let-set! fill! string-fill! list-fill! vector-fill! 
			     reverse! sort! set-car! set-cdr!)
		       (let ((sym (if (symbol? (cadr tree))
				      (cadr tree)
				      (if (pair? (cadr tree)) (caadr tree)))))
			 (if (not (or (memq sym e) (memq sym set)))
			     (set! set (cons sym set)))
			 (var-walk (cddr tree) e)))

		      ((let letrec)
		       (if (and (pair? (cdr tree))
				(pair? (cddr tree)))
			   (let* ((named (symbol? (cadr tree)))
				  (vars (if named
					    (list (shadowed (cadr tree)))
					    ())))
			     (for-each (lambda (v)
					 (when (and (pair? v) 
						    (pair? (cdr v)))
					   (var-walk (cadr v) e)
					   (set! vars (cons (shadowed (car v)) vars))))
				       ((if named caddr cadr) tree))
			     (var-walk-body ((if named cdddr cddr) tree) (append vars e)))))

		      ((case)
		       (when (and (pair? (cdr tree)) 
				  (pair? (cddr tree)))
			 (for-each (lambda (c) 
				     (when (pair? c) 
				       (var-walk (cdr c) e)))
				   (cddr tree))))

		      ((quote) #f)
		      ((let* letrec*)
		       (let* ((named (symbol? (cadr tree)))
			      (vars (if named (list (cadr tree)) ())))
			 (for-each (lambda (v)
				     (when (and (pair? v) 
						(pair? (cdr v)))
				       (var-walk (cadr v) (append vars e))
				       (set! vars (cons (shadowed (car v)) vars))))
				   ((if named caddr cadr) tree))
			 (var-walk-body ((if named cdddr cddr) tree) (append vars e))))

		      ((do)
		       (let ((vars ()))
			 (when (pair? (cadr tree))
			   (for-each (lambda (v)
				       (when (and (pair? v) 
						  (pair? (cdr v)))
					 (var-walk (cadr v) e)
					 (set! vars (cons (shadowed (car v)) vars))))
				     (cadr tree))
			   (for-each (lambda (v)
				       (if (and (pair? v)
						(pair? (cdr v))
						(pair? (cddr v)))
					   (var-walk (caddr v) (append vars e))))
				     (cadr tree)))
			 (when (pair? (cddr tree))
			   (var-walk (caddr tree) (append vars e))
			   (var-walk-body (cdddr tree) (append vars e)))))

		      ((lambda lambda*)
		       (var-walk-body (cddr tree) (append (args->proper-list (cadr tree)) e)))

		      ((define* define-macro define-macro* define-bacro define-bacro*)
		       (if (and (pair? (cdr tree))
				(pair? (cddr tree)))
			   (begin
			     (set! e (cons (caadr tree) e))
			     (var-walk-body (cddr tree) (append (args->proper-list (cdadr tree)) e)))))

		      ((define define-constant)
		       (if (and (pair? (cdr tree))
				(pair? (cddr tree)))
			   (if (symbol? (cadr tree))
			       (begin
				 (var-walk (caddr tree) e)
				 (set! e (cons (cadr tree) e)))
			       (begin
				 (set! e (cons (caadr tree) e))
				 (var-walk-body (cddr tree) (append (args->proper-list (cdadr tree)) e))))))
		      (else
		       (var-walk (car tree) e)
		       (var-walk (cdr tree) e))))))
	  e)
	(list ref set)))

    (define (get-side-effect v)
      (let ((ftype (var-ftype v)))
	(or (not (memq ftype '(define define* lambda lambda*)))
	    (let ((body (cddr (var-initial-value v)))
		  (env (var-env v))
		  (args (cons (var-name v) (args->proper-list (var-arglist v)))))
	      (let ((outvars (append (cadr (out-vars (var-name v) args body)) args)))
		(any? (lambda (f) 
			(side-effect-with-vars? f env outvars))
		      body))))))

    (define (last-par x)
      (let ((len (length x)))
	(and (positive? len)
	     (x (- len 1)))))
    
    (define* (make-fvar name ftype arglist decl initial-value env)
      (let ((new (let ((old (hash-table-ref other-identifiers name)))
		   (cons name 
			 (inlet 'signature ()
				'side-effect ()
				'allow-other-keys (and (pair? arglist)
						       (memq ftype '(define* define-macro* define-bacro* defmacro*))
						       (eq? (last-par arglist) :allow-other-keys))
				'scope ()
				'setters ()
				'env env
				'initial-value initial-value
				'values (and (pair? initial-value) (count-values (cddr initial-value)))
				'leaves #f
				'match-list #f
				'decl decl
				'arglist arglist
				'ftype ftype
				'history (if old 
					     (begin
					       (hash-table-set! other-identifiers name #f)
					       (if initial-value (cons initial-value old) old))
					     (if initial-value (list initial-value) ()))
				'set 0 
				'ref (if old (length old) 0))))))
	(when (and *report-function-stuff*
		   (not (memq name '(:lambda :dilambda)))
		   (memq ftype '(define lambda define* lambda*))
		   (pair? (caddr initial-value)))
	  (hash-table-set! equable-closures (caaddr initial-value)
			   (cons new (or (hash-table-ref equable-closures (caaddr initial-value)) ()))))
	new))
    
    (define (return-type sym e)
      (let ((sig (arg-signature sym e)))
	(and (pair? sig)
	     (or (eq? (car sig) 'values) ; turn it into #t for now
		 (car sig)))))           ; this might be undefined in the current context (eg oscil? outside clm)
    
    (define any-macro?
      (let ((macros (let ((h (make-hash-table)))
		      (for-each 
		       (lambda (m)
			 (set! (h m) #t))
		       '(call-with-values let-values define-values let*-values cond-expand require quasiquote 
					  multiple-value-bind reader-cond match while))
		      h)))
	(lambda (f env)
	  (or (hash-table-ref macros f)
	      (let ((fd (var-member f env)))
		(and (var? fd)
		     (memq (var-ftype fd) '(define-macro define-macro* define-expansion 
					    define-bacro define-bacro* defmacro defmacro* define-syntax))))))))
    (define (any-procedure? f env)
      (or (hash-table-ref built-in-functions f)
	  (let ((v (var-member f env)))
	    (and (var? v)
		 (memq (var-ftype v) '(define define* lambda lambda*))))))

    (define ->simple-type
      (let ((markers (list (cons :call/exit 'continuation?)
			   (cons :call/cc 'continuation?)
			   (cons :dilambda 'dilambda?)
			   (cons :lambda 'procedure?))))
	(lambda (c)
	  (cond ((pair? c)         'pair?)
		((integer? c)      'integer?)
		((rational? c)     'rational?)
		((real? c)	   'real?)
		((number? c)       'number?)
		((string? c)       'string?)
		((null? c)	   'null?)
		((char? c)	   'char?)
		((boolean? c)      'boolean?)
		((keyword? c)
		 (cond ((assq c markers) => cdr)
		       (else 'keyword?)))
		((vector? c)       'vector?)
		((float-vector? c) 'float-vector?)
		((int-vector? c)   'int-vector?)
		((byte-vector? c)  'byte-vector?)
		((let? c)	   'let?)
		((hash-table? c)   'hash-table?)
		((input-port? c)   'input-port?)
		((output-port? c)  'output-port?)
		((iterator? c)     'iterator?)
		((continuation? c) 'continuation?)
		((dilambda? c)     'dilambda?)
		((procedure? c)    'procedure?)
		((macro? c)        'macro?)
		((random-state? c) 'random-state?)
		((c-pointer? c)    'c-pointer?)
		((c-object? c)     'c-object?)
		((eof-object? c)   'eof-object?)
		((syntax? c)       'syntax?)
		((assq c '((#<unspecified> . unspecified?) (#<undefined> . undefined?))) => cdr)
		(#t #t)))))
    
    (define (define->type c)
      (and (pair? c)
	   (case (car c)
	     ((define)
	      (if (and (pair? (cdr c))
		       (pair? (cadr c)))
		  'procedure?
		  (and (pair? (cddr c))
		       (->lint-type (caddr c)))))
	     ((define* lambda lambda* case-lambda) 'procedure?)
	     ((dilambda) 'dilambda?)
	     ((define-macro define-macro* define-bacro define-bacro* defmacro defmacro* define-expansion) 'macro?)
	     ((:call/cc :call/exit) 'continuation?)
	     (else #t))))
    
    (define (->lint-type c)
      (cond ((not (pair? c))	        (->simple-type c))
	    ((not (symbol? (car c)))    (or (pair? (car c)) 'pair?))
	    ((not (eq? (car c) 'quote)) (or (return-type (car c) ()) (define->type c)))
	    ((symbol? (cadr c))         'symbol?)
	    (else                       (->simple-type (cadr c)))))   ; don't look for return type!
    
    (define (compatible? type1 type2) ; we want type1, we have type2 -- is type2 ok?
      (or (eq? type1 type2)
	  (not (symbol? type1))
	  (not (symbol? type2))
	  (not (hash-table-ref booleans type1))
	  (not (hash-table-ref booleans type2))
	  (eq? type2 'constant?)
	  (case type1
	    ((number? complex?)  (memq type2 '(float? real? rational? integer? number? complex? exact? inexact? zero? negative? positive? even? odd? infinite? nan?)))
	    ((real?)             (memq type2 '(float? rational? integer? complex? number? exact? inexact? zero? negative? positive? even? odd? infinite? nan?)))
	    ((zero?)             (memq type2 '(float? real? rational? integer? number? complex? exact? inexact? even?)))
	    ((negative? positive?) (memq type2 '(float? real? rational? integer? complex? number? exact? inexact? even? odd? infinite? nan?)))
	    ((float?)            (memq type2 '(real? complex? number? inexact? zero? negative? positive? infinite? nan?)))
	    ((rational?)         (memq type2 '(integer? real? complex? number? exact? zero? negative? positive? even? odd?)))
	    ((integer?)          (memq type2 '(real? rational? complex? number? exact? even? odd? zero? negative? positive?)))
	    ((odd? even?)        (memq type2 '(real? rational? complex? number? exact? integer? zero? negative? positive?)))
	    ((exact?)            (memq type2 '(real? rational? complex? number? integer? zero? negative? positive?)))
	    ((inexact?)          (memq type2 '(real? number? complex? float? zero? negative? positive? infinite? nan?)))
	    ((infinite? nan?)    (memq type2 '(real? number? complex? positive? negative? inexact? float?)))
	    ((vector?)           (memq type2 '(float-vector? int-vector? sequence?)))
	    ((float-vector? int-vector?) (memq type2 '(vector? sequence?)))
	    ((sequence?)         (memq type2 '(list? pair? null? proper-list? vector? float-vector? int-vector? byte-vector? 
					       string? let? hash-table? c-object? iterator? procedure?))) ; procedure? for extended iterator
	    ((symbol?)           (memq type2 '(gensym? keyword? defined? provided?)))
	    ((constant?)         #t)
	    ((keyword? gensym? defined? provided?)  (eq? type2 'symbol?))
	    ((list?)             (memq type2 '(null? pair? proper-list? sequence?)))
	    ((proper-list?)      (memq type2 '(null? pair? list? sequence?)))
	    ((pair? null?)       (memq type2 '(list? proper-list? sequence?)))
	    ((dilambda?)         (memq type2 '(procedure? macro? iterator?)))
	    ((procedure?)        (memq type2 '(dilambda? iterator? macro? sequence?)))
	    ((macro?)            (memq type2 '(dilambda? iterator? procedure?)))
	    ((iterator?)         (memq type2 '(dilambda? procedure? sequence?)))
	    ((string?)           (memq type2 '(byte-vector? sequence? directory? file-exists?)))
	    ((hash-table? let? c-object?) 
	     (eq? type2 'sequence?))
	    ((byte-vector? directory? file-exists?) 
	     (memq type2 '(string? sequence?)))
	    ((input-port? output-port?) 
	     (eq? type2 'boolean?))
	    ((char? char-whitespace? char-numeric? char-alphabetic? char-upper-case? char-lower-case?)
	     (memq type2 '(char? char-whitespace? char-numeric? char-alphabetic? char-upper-case? char-lower-case?)))
	    (else #f))))
    
    (define (any-compatible? type1 type2)
      ;; type1 and type2 can be either a list of types or a type
      (if (symbol? type1)
	  (if (symbol? type2)
	      (compatible? type1 type2)
	      (and (pair? type2)
		   (or (compatible? type1 (car type2))
		       (any-compatible? type1 (cdr type2)))))
	  (and (pair? type1)
	       (or (compatible? (car type1) type2)
		   (any-compatible? (cdr type1) type2)))))
    
    (define (subsumes? type1 type2)
      (or (eq? type1 type2)
	  (case type1
	    ((integer?)         (memq type2 '(even? odd?)))
	    ((rational?)        (memq type2 '(integer? exact? odd? even?)))
	    ((exact?)           (memq type2 '(integer? rational?)))
	    ((real?)            (memq type2 '(integer? rational? float? negative? positive? zero? odd? even?)))
	    ((complex? number?) (memq type2 '(integer? rational? float? real? complex? number? negative? positive? zero? 
					      even? odd? exact? inexact? nan? infinite?)))
	    ((list?)            (memq type2 '(pair? null? proper-list?)))
	    ((proper-list?)     (eq? type2 'null?))
	    ((vector?)          (memq type2 '(float-vector? int-vector?)))
	    ((symbol?)          (memq type2 '(keyword? gensym? defined? provided?)))
	    ((sequence?)        (memq type2 '(list? pair? null? proper-list? vector? float-vector? int-vector? byte-vector?
					      string? let? hash-table? c-object? directory? file-exists?)))
	    ((char?)            (memq type2 '(char-whitespace? char-numeric? char-alphabetic? char-upper-case? char-lower-case?)))
	    (else #f))))
    
    (define (never-false expr)
      (or (eq? expr #t)
	  (let ((type (if (pair? expr)
			  (return-type (car expr) ())
			  (->lint-type expr))))
	    (and (symbol? type)
		 (not (symbol? expr))
		 (not (memq type '(boolean? values)))))))
    
    (define (never-true expr)
      (or (not expr)
	  (and (pair? expr)
	       (eq? (car expr) 'not)
	       (pair? (cdr expr))
	       (never-false (cadr expr)))))
    
    (define (prettify-checker-unq op)
      (if (pair? op)
	  (string-append (prettify-checker-unq (car op)) " or " (prettify-checker-unq (cadr op)))
	  (case op
	    ((rational?) "rational")
	    ((real?) "real")
	    ((complex?) "complex")
	    ((null?) "null")
	    ((length) "a sequence")
	    ((unspecified?) "untyped")
	    ((undefined?) "not defined")
	    (else 
	     (let ((op-name (symbol->string op)))
	       (string-append (if (memv (op-name 0) '(#\a #\e #\i #\o #\u)) "an " "a ")
			      (substring op-name 0 (- (length op-name) 1))))))))
    
    (define (prettify-checker op)
      (if (pair? op)
	  (string-append (prettify-checker-unq (car op)) " or " (prettify-checker (cadr op)))
	  (let ((op-name (symbol->string op)))
	    (case op
	      ((rational? real? complex? null?) op-name)
	      ((unspecified?) "untyped")
	      ((undefined?) "not defined")
	      (else (string-append (if (memv (op-name 0) '(#\a #\e #\i #\o #\u)) "an " "a ") op-name))))))
    
    (define (side-effect-with-vars? form env vars)
      ;; (format *stderr* "~A~%" form)
      ;; could evaluation of form have any side effects (like IO etc)
      
      (if (or (not (proper-list? form))                   ; we don't want dotted lists or () here
	      (null? form))

	  (and (symbol? form)
	       (or (eq? form '=>)                         ; (cond ((x => y))...) -- someday check y...
		   (let ((e (var-member form env)))
		     (if (var? e)
			 (and (symbol? (var-ftype e))
			      (var-side-effect e))
			 (and (not (hash-table-ref no-side-effect-functions form))
			      (procedure? (symbol->value form *e*))))))) ; i.e. function passed as argument

	  ;; can't optimize ((...)...) because the car might eval to a function
	  (or (and (not (hash-table-ref no-side-effect-functions (car form)))
		   ;; if it's not in the no-side-effect table and ...
		   
		   (let ((e (var-member (car form) env)))
		     (or (not (var? e))
			 (not (symbol? (var-ftype e)))
			 (var-side-effect e)))
		   
		   (or (not (eq? (car form) 'format))                         ; (format #f ...)
		       (not (pair? (cdr form)))                               ; (format)!
		       (cadr form))

		   (or (null? vars)
		       (not (memq (car form) '(set! 
					       ;vector-set! list-set! hash-table-set! float-vector-set! int-vector-set! string-set! let-set! 
					       ;fill! string-fill! list-fill! vector-fill! 
					       ;reverse! sort!
					       define define* define-macro define-macro* define-bacro define-bacro*)))))
	      ;; it's not the common (format #f ...) special case, then...(goto case below) 
	      ;; else return #t: side-effects are possible -- this is too hard to read
	      
	      (case (car form)

		((define-constant define-expansion) #t)

		((define define* define-macro define-macro* define-bacro define-bacro*)
		 (null? vars))

		((set! 
		  ;vector-set! list-set! hash-table-set! float-vector-set! int-vector-set! string-set! let-set! 
		  ;fill! string-fill! list-fill! vector-fill! 
		  ;reverse! sort!
		  )
		 (or (not (pair? (cdr form)))
		     (not (symbol? (cadr form)))
		     (memq (cadr form) vars)))
		
		((quote) #f)
		
		((case)
		 (or (not (pair? (cdr form)))
		     (side-effect-with-vars? (cadr form) env vars) ; the selector
		     (let case-effect? ((f (cddr form)))
		       (and (pair? f)
			    (or (not (pair? (car f)))
				(any? (lambda (ff) (side-effect-with-vars? ff env vars)) (cdar f))
				(case-effect? (cdr f)))))))
		
		((cond)
		 (or (not (pair? (cadr form)))
		     (let cond-effect? ((f (cdr form))
					(e env))
		       (and (pair? f)
			    (or (and (pair? (car f)) 
				     (any? (lambda (ff) (side-effect-with-vars? ff e vars)) (car f)))
				(cond-effect? (cdr f) e))))))
		
		((let let* letrec letrec*)
		 ;; here if the var value involves a member of vars, we have to add it to vars
		 (or (< (length form) 3)
		     (let ((syms (cadr form))
			   (body (cddr form)))
		       (when (symbol? (cadr form))
			 (set! syms (caddr form))
			 (set! body (cdddr form)))
		       (if (and (pair? vars)
				(pair? syms))
			   (for-each (lambda (sym)
				       (when (and (pair? sym)
						  (pair? (cdr sym))
						  (tree-set-member vars (cdr sym)))
					 (set! vars (cons (car sym) vars))))
				     syms))
		       (or (let let-effect? ((f syms) (e env) (v vars))
			     (and (pair? f)
				  (or (not (pair? (car f)))
				      (not (pair? (cdar f))) ; an error, reported elsewhere: (let ((x)) x)
				      (side-effect-with-vars? (cadar f) e v)
				      (let-effect? (cdr f) e v))))
			   (any? (lambda (ff) (side-effect-with-vars? ff env vars)) body)))))
		
		((do)
		 (or (< (length form) 3)
		     (not (list? (cadr form)))
		     (not (list? (caddr form)))
		     (let do-effect? ((f (cadr form))	(e env))
		       (and (pair? f)
			    (or (not (pair? (car f)))
				(not (pair? (cdar f)))
				(side-effect-with-vars? (cadar f) e vars)
				(and (pair? (cddar f))
				     (side-effect-with-vars? (caddar f) e vars))
				(do-effect? (cdr f) e))))
		     (any? (lambda (ff) (side-effect-with-vars? ff env vars)) (caddr form))
		     (any? (lambda (ff) (side-effect-with-vars? ff env vars)) (cdddr form))))
		
		;; ((lambda lambda*) (any? (lambda (ff) (side-effect-with-vars? ff env vars)) (cddr form))) ; this is trickier than it looks
		
		(else
		 ;(format *stderr* "check args: ~A~%" form)
		 (or (any? (lambda (f)                                   ; any subform has a side-effect
			     (and (not (null? f))
				  (side-effect-with-vars? f env vars)))
			   (cdr form))
		     (let ((sig (procedure-signature (car form))))       ; sig has func arg and it is not known safe
		       (and (pair? sig)
			    (memq 'procedure? (cdr sig))
			    (call-with-exit
			     (lambda (return)
			       (for-each
				(lambda (sg arg)
				  (when (and (eq? sg 'procedure?)
					     (not (and (symbol? arg)
						       (hash-table-ref no-side-effect-functions arg))))
				    (return #t)))
				(cdr sig) (cdr form))
			       #f))))))))))
	  
    
    (define (side-effect? form env)
      (side-effect-with-vars? form env ()))

    (define (just-constants? form env)
      ;; can we probably evaluate form given just built-in stuff?
      ;;   watch out here -- this is used later by 'if, so (defined 'hiho) should not be evalled to #f!
      (if (not (pair? form))
	  (constant? form)
	  (and (symbol? (car form))
	       (hash-table-ref no-side-effect-functions (car form))
	       (hash-table-ref built-in-functions (car form)) ; and not hook-functions
	       (not (var-member (car form) env)) ; e.g. exp declared locally as a list
	       (every? (lambda (p) (just-constants? p env)) (cdr form)))))

    (define (equal-ignoring-constants? a b)
      (or (morally-equal? a b)
	  (and (symbol? a)
	       (constant? a) 
	       (morally-equal? (symbol->value a) b))
	  (and (symbol? b)
	       (constant? b)
	       (morally-equal? (symbol->value b) a))
	  (and (pair? a)
	       (pair? b)
	       (equal-ignoring-constants? (car a) (car b))
	       (equal-ignoring-constants? (cdr a) (cdr b)))))
    
    
    (define (repeated-member? lst env)
      (and (pair? lst)
	   (or (and (not (and (pair? (car lst))
			      (side-effect? (car lst) env)))
		    (pair? (cdr lst))
		    (member (car lst) (cdr lst)))
	       (repeated-member? (cdr lst) env))))

    (define (update-scope v caller env)
      (unless (or (memq caller (var-scope v))
		  (assq caller (var-scope v)))
	(let ((cv (var-member caller env)))
	  (set! (var-scope v) 
		(cons (if (and (var? cv)
			       (memq (var-ftype cv) '(define lambda define* lambda*))) ; named-let does not define ftype
			  caller
			  (cons caller env))
		      (var-scope v))))))

    
    (define (check-for-bad-variable-name caller vname)
      (define (bad-variable-name-numbered vname bad-names)
	(let ((str (symbol->string vname)))
	  (let loop ((bads bad-names))
	    (and (pair? bads)
		 (let* ((badstr (symbol->string (car bads)))
			(pos (string-position badstr str)))
		   (or (and (eqv? pos 0)
			    (string->number (substring str (length badstr))))
		       (loop (cdr bads))))))))
      (if (and (symbol? vname)
	       (pair? *report-bad-variable-names*)
	       (or (memq vname *report-bad-variable-names*)
		   (let ((sname (symbol->string vname)))
		     (and (> (length sname) 8)
			  (or (string=? "compute" (substring sname 0 7))      ; compute-* is as bad as get-*
			      (string=? "calculate" (substring sname 0 9))))) ;   perhaps one exception: computed-goto*
		   (bad-variable-name-numbered vname *report-bad-variable-names*)))
	  (lint-format "surely there's a better name for this variable than ~A" caller vname)))

    (define (set-ref name caller form env)
      ;; if name is in env, set its "I've been referenced" flag
      (let ((data (var-member name env)))
	(if (var? data)
	    (begin
	      (set! (var-ref data) (+ (var-ref data) 1))
	      (update-scope data caller env)
	      (if (and form (not (memq form (var-history data))))
		  (set! (var-history data) (cons form (var-history data)))))
	    (if (not (defined? name (rootlet)))
		(let ((old (hash-table-ref other-identifiers name)))
		  (check-for-bad-variable-name caller name)
		  (hash-table-set! other-identifiers name (if old (cons form old) (list form)))))))
      env)

    
    (define (set-set name caller form env)
      (let ((data (var-member name env)))
	(when (var? data)
	  (set! (var-set data) (+ (var-set data) 1))
	  (update-scope data caller env)
	  (if (not (memq caller (var-setters data)))
	      (set! (var-setters data) (cons caller (var-setters data))))
	  (if (not (memq form (var-history data)))
	      (set! (var-history data) (cons form (var-history data))))
	  (set! (var-signature data) #f)
	  (set! (var-ftype data) #f))))
    
    
    (define (proper-list lst)
      ;; return lst as a proper list
      (if (not (pair? lst))
	  lst
	  (cons (car lst) 
		(if (pair? (cdr lst)) 
		    (proper-list (cdr lst)) 
		    (if (null? (cdr lst)) 
			() 
			(list (cdr lst)))))))
    
    (define (keywords lst)
      (do ((count 0)
	   (p lst (cdr p)))
	  ((null? p) count)
	(if (keyword? (car p))
	    (set! count (+ count 1)))))
    
    (define (eqv-selector clause)
      (if (not (pair? clause))
	  (memq clause '(else #t))
	  (case (car clause)
	    ((memq memv member) 
	     (and (= (length clause) 3)
		  (cadr clause)))
	    ((eq? eqv? = equal? char=? char-ci=? string=? string-ci=?)
	     (and (= (length clause) 3)
		  ((if (code-constant? (cadr clause)) caddr cadr) clause)))
	    ((or) 
	     (and (pair? (cdr clause))
		  (eqv-selector (cadr clause))))
	    ((not null? eof-object? zero? boolean?)
	     (and (pair? (cdr clause))
		  (cadr clause)))
 	    (else #f))))
    
    (define (->eqf x)
      (case x
	((char?) '(eqv? char=?))
	((integer? rational? real? number? complex?) '(eqv? =))
	((symbol? keyword? boolean? null? procedure? syntax? macro? undefined? unspecified?) '(eq? eq?))
	((string? byte-vector?) '(equal? string=?))
	((pair? vector? float-vector? int-vector? hash-table?) '(equal? equal?))
	((eof-object?) '(eq? eof-object?))
	(else 
	 (if (and (pair? x)
		  (pair? (cdr x))
		  (null? (cddr x))
		  (or (and (memq 'boolean? x)
			   (or (memq 'real? x) (memq 'number? x) (memq 'integer? x)))
		      (and (memq 'eof-object? x)
			   (or (memq 'char? x) (memq 'integer? x)))))
	     '(eqv? eqv?)
	     '(#t #t)))))
    
    (define (eqf selector env)
      (cond ((symbol? selector) 
	     (if (and (not (var-member selector env))
		      (or (hash-table-ref built-in-functions selector)
			  (hash-table-ref syntaces selector)))
		 '(eq? eq?)
		 '(#t #t)))

	    ((not (pair? selector)) 
	     (->eqf (->lint-type selector)))

	    ((eq? (car selector) 'quote)
	     (cond ((or (symbol? (cadr selector))
			(memq (cadr selector) '(#f #t #<unspecified> #<undefined> #<eof> ())))
		    '(eq? eq?))
		   ((char? (cadr selector))   '(eqv? char=?))
		   ((string? (cadr selector)) '(equal? string=?))
		   ((number? (cadr selector)) '(eqv? =))
		   (else                      '(equal? equal?))))

	    ((and (eq? (car selector) 'list)
		  (null? (cdr selector)))
	     '(eq? eq?))

	    ((symbol? (car selector)) 
	     (let ((sig (arg-signature (car selector) env)))
	       (if (pair? sig)
		   (->eqf (car sig))
		   '(#t #t))))

	    (else '(#t #t))))
    
    (define (unquoted x)
      (if (and (pair? x)
	       (eq? (car x) 'quote))
	  (cadr x)
	  x))
    
    (define (distribute-quote x)
      (map (lambda (item)
	     (if (or (symbol? item)
		     (pair? item))
		 `(quote ,item)
		 item))
	   x))
    
    (define (focus-str str focus)
      (let ((len (length str)))
	(if (< len 40)
	    str
	    (let ((pos (string-position focus str))
		  (focus-len (length focus)))
	      (if (not pos)
		  str
		  (if (<= pos 20)
		      (string-append (substring str 0 (min 60 (- len 1) (+ focus-len pos 20))) " ...")
		      (string-append "... " (substring str (- pos 20) (min (- len 1) (+ focus-len pos 20))) " ...")))))))
    
    (define (check-star-parameters f args env)
      (if (list-any? (lambda (k) (memq k '(:key :optional))) args)
	  (let ((kw (if (memq :key args) :key :optional)))
	    (format outport "~NC~A: ~A is no longer accepted: ~A~%" lint-left-margin #\space f kw 
		    (focus-str (object->string args) (symbol->string kw)))))
      
      (if (member 'pi args (lambda (a b) (or (eq? b 'pi) (and (pair? b) (eq? (car b) 'pi)))))
	  (format outport "~NC~A: parameter can't be a constant: ~A~%" lint-left-margin #\space f 
		  (focus-str (object->string args) "pi")))
      
      (let ((r (memq :rest args)))
	(when (pair? r)
	  (if (not (pair? (cdr r)))
	      (format outport "~NC~A: :rest parameter needs a name: ~A~%" lint-left-margin #\space f args)
	      (if (pair? (cadr r))
		  (format outport "~NC~A: :rest parameter can't specify a default value: ~A~%" lint-left-margin #\space f args)))))
      
      (let ((a (memq :allow-other-keys args)))
	(if (and (pair? a)
		 (pair? (cdr a)))
	    (format outport "~NC~A: :allow-other-keys should be at the end of the parameter list: ~A~%" lint-left-margin #\space f 
		    (focus-str (object->string args) ":allow-other-keys"))))

      (for-each (lambda (p)
		  (if (and (pair? p)
			   (pair? (cdr p)))
		      (lint-walk f (cadr p) env)))
		args))
    
    (define (checked-eval form)
      (and (proper-list? form) ;(not (infinite? (length form))) but when would a dotted list work?
	   (catch #t
	     (lambda ()
	       (eval (copy form :readable)))
	     (lambda args
	       :checked-eval-error))))
    
    (define (return-type-ok? type ret)
      (or (eq? type ret)
	  (and (pair? ret)
	       (memq type ret))))

    
    (define last-and-incomplete-arg2 #f)

    (define (and-incomplete form head arg1 arg2 env)           ; head: 'and | 'or (not ...) | 'if | 'if2 -- symbol arg1 in any case
      (unless (memq (car arg2) '(and or not list cons vector)) ; these don't tell us anything about arg1's type
	(let ((v (var-member arg1 env)))                       ; try to avoid the member->cdr trope
	  (unless (or (eq? arg2 last-and-incomplete-arg2)
		      (and (var? v)
			   (pair? (var-history v))
			   (member #f (var-history v)
				   (lambda (a b)
				     (and (pair? b)
					  (memq (car b) '(char-position string-position format string->number assoc assq assv memq memv member)))))))
	    (let* ((pos (do ((i 0 (+ i 1))                     ; get arg number of arg1 in arg2
			     (p arg2 (cdr p))) ; 0th=car -> (and x (x))
			    ((or (null? p)
				 (eq? (car p) arg1))
			     i)))
		   (arg-type (let ((sig (and (positive? pos)   ; procedure-signature for arg2
					     (arg-signature (car arg2) env))))
			       (if (zero? pos)                 ; it's type indication for arg1's position
				   'procedure? ; or sequence? -- how to distinguish? use 'applicable?
				   (and (pair? sig)
					(< pos (length sig))
					(list-ref sig pos))))))
	      (let ((ln (and (< 0 line-number 100000) line-number))
		    (comment (if (and (eq? arg-type 'procedure?)
				      (= pos 0)
				      (pair? (cdr arg2)))
				 " ; or maybe sequence? " "")))
		
		(set! last-and-incomplete-arg2 arg2)             ; ignore unwanted repetitions due to recursive simplifications
		(if (symbol? arg-type)
		    (let ((old-arg (case head
				     ((and if cond when) arg1)
				     ((or if2)           `(not ,arg1))))
			  (new-arg (case head
				     ((and if cond when) `(,arg-type ,arg1))
				     ((or if2)           `(not (,arg-type ,arg1))))))
		      (format outport "~NCin ~A~A,~%~NCperhaps change ~S to ~S~A~%"
			      lint-left-margin #\space 
			      (truncated-list->string form)
			      (if ln (format #f " (line ~D)" ln) "")
			      (+ lint-left-margin 4) #\space 
			      old-arg new-arg comment)))))))))

    (define (and-redundant? arg1 arg2)
      (let ((type1 (car arg1))
	    (type2 (car arg2)))
	(and (symbol? type1)
	     (symbol? type2)
	     (hash-table-ref booleans type1)
	     (or (hash-table-ref booleans type2)     ; return #f if not (obviously) redundant, else return which of the two to keep
		 (memq type2 '(= char=? string=? not eq?)))
	     (if (eq? type1 type2)
		 type1
		 (case type1
		   ((number? complex?) 
		    (case type2
		      ((float? real? rational? integer?) type2)
		      ((number? complex?) type1)
		      ((=) (let ((x ((if (number? (caddr arg2)) caddr cadr) arg2)))
			     (and (number? x)
				  (if (= x (floor x)) 'memv 'eqv?))))
		      (else #f)))
		   
		   ((real?)
		    (case type2
		      ((float? rational? integer?) type2)
		      ((number? complex?) type1)
		      ((=) (let ((x ((if (real? (caddr arg2)) caddr cadr) arg2)))
			     (and (real? x)
				  (if (= x (floor x)) 'memv 'eqv?))))
		      (else #f)))
		   
		   ((float?)           
		    (and (memq type2 '(real? complex? number? inexact?)) type1))
		   
		   ((rational?)
		    (case type2
		      ((integer?) type2)
		      ((real? complex? number? exact?) type1)
		      ((=)
		       (and (or (rational? (caddr arg2))
				(rational? (cadr arg2)))
			    'eqv?))
		      (else #f)))
		   
		   ((integer?)
		    (case type2
		      ((real? rational? complex? number? exact?) type1)
		      ((=)
		       (and (or (integer? (caddr arg2))
				(integer? (cadr arg2)))
			    'eqv?))
		      (else #f)))
		   
		   ((exact?)           
		    (and (memq type2 '(rational? integer?)) type2))
		   
		   ((even? odd?)       
		    (and (memq type2 '(integer? rational? real? complex? number?)) type1)) ; not zero? -> 0.0
		   
		   ((zero?)            
		    (and (memq type2 '(complex? number? real?)) type1))
		   
		   ((negative? positive?) 
		    (and (eq? type2 'real?) type1))
		   
		   ((inexact?)         
		    (and (eq? type2 'float?) type2))
		   
		   ((infinite? nan?)   
		    (and (memq type2 '(number? complex? inexact?)) type1))
		   
		   ((vector?)          
		    (and (memq type2 '(float-vector? int-vector?)) type2))
		   
		   ((float-vector? int-vector?) 
		    (and (eq? type2 'vector?) type1))
		   
		   ((symbol?) 
		    (case type2
		      ((keyword? gensym?) type2)
		      ((eq?)
		       (and (or (quoted-symbol? (cadr arg2))
				(quoted-symbol? (caddr arg2)))
			    'eq?))
		      (else #f)))
		   
		   ((keyword?)
		    (case type2
		      ((symbol? constant?) type1)
		      ((eq?)
		       (and (or (keyword? (cadr arg2))
				(keyword? (caddr arg2)))
			    'eq?))
		      (else #f)))
		   
		   ((gensym? defined? provided?) 
		    (and (eq? type2 'symbol?) type1))
		   
		   ((boolean?)         
		    (and (or (eq? type2 'not) 
			     (and (eq? type2 'eq?)
				  (or (boolean? (cadr arg2))
				      (boolean? (caddr arg2)))))
			 
			 type2))
		   
		   ((list?)            
		    (and (memq type2 '(null? pair? proper-list?)) type2))
		   
		   ((null?)            
		    (and (memq type2 '(list? proper-list?)) type1))
		   
		   ((pair?)            
		    (and (eq? type2 'list?) type1))
		   
		   ((proper-list?)     
		    (and (eq? type2 'null?) type2))
		   
		   ((string?)
		    (case type2
		      ((byte-vector?) type2)
		      ((string=?)
		       (and (or (eq? (->lint-type (cadr arg2)) 'string?)
				(eq? (->lint-type (caddr arg2)) 'string?))
			    'equal?))
		      (else #f)))
		   
		   ((char?)            
		    (and (eq? type2 'char=?)
			 (or (eq? (->lint-type (cadr arg2)) 'char?)
			     (eq? (->lint-type (caddr arg2)) 'char?))
			 'eqv?))
		   
		   ((char-numeric? char-whitespace? char-alphabetic? char-upper-case? char-lower-case?) 
		    (and (eq? type2 'char?) type1))
		   
		   ((byte-vector? directory? file-exists?) 
		    (and (eq? type2 'string?) type1))
		   
		   (else #f))))))
    
    
    (define (and-forgetful form head arg1 arg2 env)
      (unless (or (memq (car arg2) '(and or not list cons vector)) ; these don't tell us anything about arg1's type
		  (eq? arg2 last-and-incomplete-arg2))
	(let* ((pos (do ((i 0 (+ i 1))                         ; get arg number of arg1 in arg2
			 (p arg2 (cdr p))) ; 0th=car -> (and x (x))
			((or (null? p)
			     (equal? (car p) (cadr arg1)))
			 (if (null? p) -1 i))))
	       (arg-type (let ((sig (and (positive? pos)       ; procedure-signature for arg2
					 (arg-signature (car arg2) env))))
			   (if (zero? pos)                     ; its type indication for arg1's position
			       'procedure?                     ; or sequence? -- how to distinguish? use 'applicable?
			       (and (pair? sig)
				    (< pos (length sig))
				    (list-ref sig pos))))))
	  (when (symbol? arg-type)
	    (let ((new-type (and-redundant? arg1 (cons arg-type (cdr arg1)))))
	      (when (and new-type
			 (not (eq? new-type (car arg1))))
		(let ((old-arg (case head
				 ((and if cond when) arg1)
				 ((or if2)           `(not ,arg1))))
		      (new-arg (case head
				 ((and if cond when) `(,new-type ,(cadr arg1)))
				 ((or if2)           `(not (,new-type ,(cadr arg1))))))
		      (ln (and (< 0 line-number 100000) line-number))
		      (comment (if (and (eq? arg-type 'procedure?)
					(= pos 0)
					(pair? (cdr arg2)))
				   " ; or maybe sequence? " "")))
		  (set! last-and-incomplete-arg2 arg2)
		  (format outport "~NCin ~A~A,~%~NCperhaps change ~S to ~S~A~%"
			  lint-left-margin #\space 
			  (truncated-list->string form)
			  (if ln (format #f " (line ~D)" ln) "")
			  (+ lint-left-margin 4) #\space 
			  old-arg new-arg comment)))))))
      
      ;; perhaps change pair? -> eq? or ignore it?
      (when (and (pair? (cdr arg2))
		 (not (eq? (car arg1) 'pair?)))
	(let ((a2 (if (eq? (car arg2) 'not)
		      (cadr arg2)
		      arg2)))
	  (when (and (pair? a2)
		     (memq (car a2) '(memq memv member assq assv assoc eq? eqv? equal?))
		     (equal? (cadr arg1) (cadr a2)))
	    (let ((new-e (case (car (->eqf (car arg1)))
			   ((eq?)
			    (case (car a2)
			      ((memq assq eq?) (car a2))
			      ((memv member) 'memq)
			      ((assv assoc) 'assq)
			      ((eqv? equal?) 'eq?)))
			   ((eqv?)
			    (case (car a2)
			      ((memv assv eqv?) (car a2))
			      ((memq member) 'memv)
			      ((assq assoc) 'assv)
			      ((eq? equal?) 'eqv?)))
			   ((equal?)
			    (case (car a2)
			      ((member assoc equal?) (car a2))
			      ((memq memv) 'member)
			      ((assq assv) 'assoc)
			      ((eq? eqv?) 'equal?)))
			   (else (car a2)))))
	      (when (and (not (eq? (car a2) new-e))
			 (symbol? new-e))
		(let ((ln (and (< 0 line-number 100000) line-number)))
		  (format outport "~NCin ~A~A,~%~NCperhaps change ~A to ~A~%"
			  lint-left-margin #\space 
			  (truncated-list->string form)
			  (if ln (format #f " (line ~D)" ln) "")
			  (+ lint-left-margin 4) #\space 
			  (truncated-list->string a2)
			  `(,new-e ...)))))))))


    ;; --------------------------------
    (define simplify-boolean

      (let ((notables (let ((h (make-hash-table)))
		    (for-each
		     (lambda (op)
		       (set! (h (car op)) (cadr op)))
		     '((< >=) (> <=) (<= >) (>= <)
		       (char<? char>=?) (char>? char<=?) (char<=? char>?) (char>=? char<?)
		       (string<? string>=?) (string>? string<=?) (string<=? string>?) (string>=? string<?)
		       (char-ci<? char-ci>=?) (char-ci>? char-ci<=?) (char-ci<=? char-ci>?) (char-ci>=? char-ci<?)
		       (string-ci<? string-ci>=?) (string-ci>? string-ci<=?) (string-ci<=? string-ci>?) (string-ci>=? string-ci<?)
		       (odd? even?) (even? odd?) (exact? inexact?) (inexact? exact?)))
		    h))
	    (relsub
	     (let ((relops '((< <= > number?) (<= < >= number?) (> >= < number?) (>= > <= number?)
			     (char<? char<=? char>? char?) (char<=? char<? char>=? char?)  ; these never happen
			     (char>? char>=? char<? char?) (char>=? char>? char<=? char?)
			     (string<? string<=? string>? string?) (string<=? string<? string>=? string?)
			     (string>? string>=? string<? string?) (string>=? string>? string<=? string?))))
	       (lambda (A B rel-op env)
		 (call-with-exit
		  (lambda (return)
		    (when (and (pair? A)
			       (pair? B)
			       (= (length A) (length B) 3))
		      (let ((Adata (assq (car A) relops))
			    (Bdata (assq (car B) relops)))
			(when (and Adata Bdata)
			  (let ((op1 (car A))
				(op2 (car B))
				(A1 (cadr A))
				(A2 (caddr A))
				(B1 (cadr B))
				(B2 (caddr B)))
			    (let ((x (if (and (not (number? A1))
					      (member A1 B))
					 A1 
					 (and (not (number? A2))
					      (member A2 B) 
					      A2))))
			      (when x
				(let ((c1 (if (equal? x A1) A2 A1))
				      (c2 (if (equal? x B1) B2 B1))
				      (type (cadddr Adata)))
				  (if (or (side-effect? c1 env)
					  (side-effect? c2 env)
					  (side-effect? x env))
				      (return 'ok))
				  (if (equal? x A2) (set! op1 (caddr Adata)))
				  (if (equal? x B2) (set! op2 (caddr Bdata)))
				  
				  (let ((typer #f)
					(gtes #f)
					(gts #f)
					(eqop #f))
				    (case type
				      ((number?)
				       (set! typer number?)
				       (set! gtes '(>= <=))
				       (set! gts  '(< >))
				       (set! eqop '=))
				      ((char?)
				       (set! typer char?)
				       (set! gtes '(char>=? char<=?))
				       (set! gts  '(char<? char>?))
				       (set! eqop 'char=?))
				      ((string?)
				       (set! typer string?)
				       (set! gtes '(string>=? string<=?))
				       (set! gts  '(string<? string>?))
				       (set! eqop 'string=?)))
				    
				    (case rel-op
				      ((and)
				       (cond ((equal? c1 c2)
					      (cond ((eq? op1 op2)
						     (return `(,op1 ,x ,c1)))
						    
						    ((eq? op2 (cadr (assq op1 relops)))
						     (return `(,(if (memq op2 gtes) op1 op2) ,x ,c1)))
						    
						    ((and (memq op1 gtes)
							  (memq op2 gtes))
						     (return `(,eqop ,x ,c1)))
						    
						    (else (return #f))))
					     
					     ((and (typer c1)
						   (typer c2))
					      (cond ((or (eq? op1 op2)
							 (eq? op2 (cadr (assq op1 relops))))
						     (return (if ((symbol->value op1) c1 c2)
								 `(,op1 ,x ,c1)
								 `(,op2 ,x ,c2))))
						    ((eq? op1 (caddr (assq op2 relops)))
						     (if ((symbol->value op1) c2 c1)
							 (return `(,op1 ,c2 ,x ,c1))
							 (if (memq op1 gts)
							     (return #f))))
						    ((and (eq? op2 (hash-table-ref reversibles (cadr (assq op1 relops))))
							  ((symbol->value op1) c1 c2))
						     (return #f))))

					     ((eq? op2 (caddr (assq op1 relops)))
					      (return `(,op1 ,c2 ,x ,c1)))))
				      
				      ((or)
				       (cond ((equal? c1 c2)
					      (cond ((eq? op1 op2)
						     (return `(,op1 ,x ,c1)))
						    
						    ((eq? op2 (cadr (assq op1 relops)))
						     (return `(,(if (memq op2 gtes) op2 op1) ,x ,c1)))
						    
						    ((and (memq op1 gts)
							  (memq op2 gts))
						     (return `(not (,eqop ,x ,c1))))
						    
						    (else (return #t))))
					     
					     ((and (typer c1)
						   (typer c2))
					      (cond ((or (eq? op1 op2)
							 (eq? op2 (cadr (assq op1 relops))))
						     (return (if ((symbol->value op1) c1 c2) 
								 `(,op2 ,x ,c2)
								 `(,op1 ,x ,c1))))
						    ((eq? op1 (caddr (assq op2 relops)))
						     (if ((symbol->value op1) c2 c1)
							 (return #t))
						     (return `(not (,(cadr (assq op1 relops)) ,c1 ,x ,c2))))
						    ((and (eq? op2 (hash-table-ref reversibles (cadr (assq op1 relops))))
							  ((symbol->value op1) c2 c1))
						     (return #t))))

					     ((eq? op2 (caddr (assq op1 relops)))
					      (return `(not (,(cadr (assq op1 relops)) ,c1 ,x ,c2)))))))))))))))
		    'ok))))))

	(lambda (in-form true false env)
    
      (define (classify e)
	(if (not (just-constants? e env))
	    e
	    (catch #t
	      (lambda ()
		(let ((val (eval e)))
		  (if (boolean? val)
		      val
		      e)))
	      (lambda ignore e))))
      
      (define (contradictory? ands)
	(let ((vars ()))
	  (call-with-exit
	   (lambda (return)
	     (do ((b ands (cdr b)))
		 ((null? b) #f)
	       (if (and (pair? b)
			(pair? (car b))
			(pair? (cdar b)))
		   (let ((func (caar b))
			 (args (cdar b)))
		     
		     (if (memq func '(eq? eqv? equal?))
			 (if (and (symbol? (car args))
				  (code-constant? (cadr args)))
			     (set! func (->lint-type (cadr args)))
			     (if (and (symbol? (cadr args))
				      (code-constant? (car args)))
				 (set! func (->lint-type (car args))))))
		     
		     (if (symbol? func)
			 (for-each
			  (lambda (arg)
			    (if (symbol? arg)
				(let ((type (assq arg vars)))
				  (if (not type)
				      (set! vars (cons (cons arg func) vars))
				      (if (not (compatible? (cdr type) func))
					  (return #t))))))
			  args)))))))))
      
      (define (and-redundants env . args)
	(do ((locals ())
	     (diffs #f)
	     (p args (cdr p)))
	    ((or (null? p)
		 (not (and (pair? (car p))
			   (pair? (cdar p))
			   (hash-table-ref booleans (caar p)))))
	     (and (null? p)
		  (pair? locals)
		  (or diffs
		      (any? (lambda (a) (pair? (cddr a))) locals))
		  (let ((keepers ()))
		    (for-each (lambda (a)
				(cond ((null? (cddr a))
				       (set! keepers (cons (cadr a) keepers)))
				      
				      ((null? (cdddr a))
				       (let ((res (apply and-redundant? (reverse (cdr a)))))
					 (if res
					     (begin
					       (set! keepers (cons ((if (eq? res (caadr a)) cadr caddr) a) keepers))
					       (set! diffs #t))
					     (set! keepers (cons (cadr a) (cons (caddr a) keepers))))))
				      
				      (else
				       (let ((ar (reverse (cdr a))))
					 (let ((res1 (and-redundant? (car ar) (cadr ar)))   ; if res1 either 1 or 2 is out
					       (res2 (and-redundant? (cadr ar) (caddr ar))) ; if res2 either 2 or 3 is out
					       (res3 (and-redundant? (car ar) (caddr ar)))) ; if res3 either 1 or 3 is out
					   ;; only in numbers can 3 actually be reducible
					   (if (not (or res1 res2 res3))
					       (set! keepers (append (cdr a) keepers))
					       (begin
						 (set! diffs #t)
						 (if (and (or (not res1)
							      (eq? res1 (caar ar)))
							  (or (not res3)
							      (eq? res3 (caar ar))))
						     (set! keepers (cons (car ar) keepers)))
						 (if (and (or (not res1)
							      (eq? res1 (caadr ar)))
							  (or (not res2)
							      (eq? res2 (caadr ar))))
						     (set! keepers (cons (cadr ar) keepers)))
						 (if (and (or (not res2)
							      (eq? res2 (car (caddr ar))))
							  (or (not res3)
							      (eq? res3 (car (caddr ar)))))
						     (set! keepers (cons (caddr ar) keepers)))
						 (if (pair? (cdddr ar))
						     (set! keepers (append (reverse (cdddr ar)) keepers))))))))))
			      (reverse locals))
		    (and diffs (reverse keepers)))))
	  (let* ((bool (car p))
		 (local (assoc (cadr bool) locals)))
	    (if (pair? local)
		(if (member bool (cdr local))
		    (set! diffs #t)
		    (set-cdr! local (cons bool (cdr local))))
		(set! locals (cons (list (cadr bool) bool) locals))))))
      
      
      (define (and-not-redundant arg1 arg2)
	(let ((type1 (car arg1))    ; (? ...)
	      (type2 (caadr arg2))) ; (not (? ...))
	  (and (symbol? type1)
	       (symbol? type2)
	       (or (hash-table-ref booleans type1)
		   (memq type1 '(= char=? string=?)))
	       (hash-table-ref booleans type2)
	       (if (eq? type1 type2)     ; (and (?) (not (?))) -> #f
		   'contradictory
		   (case type1
		     ((pair?) 
		      (case type2
			((list?) 'contradictory)
			((proper-list?) #f)
			(else arg1)))

		     ((null?) 
		      (if (eq? type2 'list?)
			  'contradictory
			  arg1))

		     ((list?) 
		      (case type2
			((pair?) 'null?)
			((null?) 'pair?)
			((proper-list?) #f)
			(else arg1)))

		     ((proper-list?) 
		      (case type2
			((list? pair?) 'contradictory)
			((null?) #f)
			(else arg1)))

		     ((symbol?) 
		      (and (not (memq type2 '(keyword? gensym?))) 
			   arg1))

		     ((char=?)  
		      (if (eq? type2 'char?)
			  'contradictory
			  (and (or (char? (cadr arg1))
				   (char? (caddr arg1)))
			       `(eqv? ,@(cdr arg1))))) ; arg2 might be (not (eof-object?...))

		     ((real?) 
		      (case type2
			((rational? exact?)  `(float? ,@(cdr arg1)))
			((inexact?)          `(rational? ,@(cdr arg1)))
			((complex? number?)  'contradictory)
			((negative? positive? even? odd? zero? integer?) #f)
			(else arg1)))

		     ((integer?) 
		      (case type2
			((real? complex? number? rational? exact?) 'contradictory)
			((float? inexact? infinite? nan?) arg1)
			(else #f)))

		     ((rational?) 
		      (case type2
			((real? complex? number? exact?) 'contradictory)
			((float? inexact? infinite? nan?) arg1)
			(else #f)))

		     ((complex? number?) 
		      (and (memq type2 '(complex? number?))
			   'contradictory))

		     ((float?) 
		      (case type2
			((real? complex? number? inexact?) 'contradictory)
			((rational? integer? exact?) arg1)
			(else #f)))

		     ((exact?) 
		      (case type2
			((rational?) 'contradictory)
			((inexact? infinite? nan?) arg1)
			(else #f)))

		     ((even? odd?) 
		      (case type2
			((integer? exact? rational? real? number? complex?) 'contradictory)
			((infinite? nan?) arg1)
			(else #f)))

		     ((zero? negative? positive?) 
		      (and (memq type2 '(complex? number? real?))
			   'contradictory))

		     ((infinite? nan?) 
		      (case type2
			((number? complex? inexact?) 'contradictory)
			((integer? rational? exact? even? odd?)	arg1)
			(else #f)))

		     ((char-whitespace? char-numeric? char-alphabetic? char-upper-case? char-lower-case?)
		      (and (eq? type2 'char?)
			   'contradictory))

		     ((directory? file-exists?)
		      (and (memq type2 '(string? sequence?))
			   'contradictory))

		     (else 
		      ;; none of the rest happen
		      #f))))))
      
      (define (or-not-redundant arg1 arg2)
	(let ((type1 (car arg1))    ; (? ...)
	      (type2 (caadr arg2))) ; (not (? ...))
	  (and (symbol? type1)
	       (symbol? type2)
	       (or (hash-table-ref bools type1)
		   (memq type1 '(= char=? string=?)))
	       (hash-table-ref bools type2)
	       (if (eq? type1 type2)     ; (or (?) (not (?))) -> #t
		   'fatuous
		   (case type1
		     ((null?) 
		      (case type2
			((list?) ; not proper-list? here
			 `(not (pair? ,(cadr arg1))))
			((proper-list?) #f)
			(else arg2)))
		     ((eof-object?) 
		      arg2) ; not keyword? here because (or (not (symbol? x)) (keyword? x)) is not reducible to (not (symbol? x))
		     ((string?) 
		      (and (not (eq? type2 'byte-vector?)) arg2))
		     (else #f))))))

      (define (bsimp x) ; quick check for common easy cases
	(set! last-simplify-boolean-line-number line-number)
	(if (not (and (pair? x)
		      (pair? (cdr x))))
	    x
	    (case (car x)
	      ((and) (and (cadr x)              ; (and #f ...) -> #f
			  x))
	      ((or) (if (and (cadr x)           ; (or #t ...) -> #t
			     (code-constant? (cadr x)))
			(cadr x)
			x))
	      (else 
	       (if (not (and (= (length x) 2)
			     (pair? (cadr x))
			     (symbol? (caadr x))))
		   x
		   (let ((rt (if (eq? (caadr x) 'quote)
				 (->simple-type (cadadr x))
				 (return-type (caadr x) env)))
			 (head (car x)))
		     (or (and (subsumes? head rt) #t) ; don't return the memq list!
			 (and (or (memq rt '(#t #f values))
				  (any-compatible? head rt))
			      (case head
				((null?) (if (eq? (caadr x) 'list)
					     (null? (cdadr x))
					     x))
				((pair?) (if (eq? (caadr x) 'list)
					     (pair? (cdadr x))
					     x))
				((negative?) (and (not (hash-table-ref non-negative-ops (caadr x)))
						  x))
				(else x))))))))))
      
      (define (bcomp x) ; not so quick...
	(cond ((not (pair? x))
	       x)

	      ((eq? (car x) 'and)
	       (call-with-exit
		(lambda (return)
		  (let ((newx (list 'and)))
		    (do ((p (cdr x) (cdr p))
			 (sidex newx)
			 (endx newx))
			((null? p) newx)
		      (let ((next (car p)))
			(if (or (not next)        ; #f in and -> end of expr
				(member next false))
			    (if (eq? sidex newx)  ; no side-effects
				(return #f)       
				(begin
				  (set-cdr! endx (list #f))
				  (return newx)))
			    (if (or (code-constant? next)  ; (and ... true-expr ...)
				    (member next sidex)    ; if a member, and no side-effects since, it must be true
				    (member next true))
				(if (and (null? (cdr p))
					 (not (equal? next (car endx))))
				    (set-cdr! endx (list next)))
				(begin
				  (set-cdr! endx (list next))
				  (set! endx (cdr endx))
				  (if (side-effect? next env)
				      (set! sidex endx)))))))))))
		
	      ((not (eq? (car x) 'or))
	       x)

	      (else
	       (call-with-exit
		(lambda (return)
		  (let ((newx (list 'or)))
		    (do ((p (cdr x) (cdr p))
			 (sidex newx)
			 (endx newx))
			((null? p) newx)
		      (let ((next (car p)))
			(if (or (and next                 ; (or ... #t ...)
				     (code-constant? next))
				(member next true))
			    (begin
			      (set-cdr! endx (list next))
			      (return newx))          ; we're done since this is true
			    (if (or (not next)
				    (member next sidex) ; so its false in some way
				    (member next false))
				(if (and (null? (cdr p))
					 (not (equal? next (car endx))))
				    (set-cdr! endx (list next)))
				(begin
				  (set-cdr! endx (list next))
				  (set! endx (cdr endx))
				  (if (side-effect? next env)
				      (set! sidex endx)))))))))))))
      
      (define (gather-or-eqf-elements eqfnc sym vals)
	(let* ((func (case eqfnc 
		       ((eq?) 'memq) 
		       ((eqv? char=?) 'memv) 
		       (else 'member)))
	       (equals (if (and (eq? func 'member)
				(not (eq? eqfnc 'equal?)))
			   (list eqfnc)
			   ()))
	       (elements (lint-remove-duplicates (map unquoted vals) env)))
	  (cond ((null? (cdr elements))
		 `(,eqfnc ,sym ,@elements))
		
		((and (eq? eqfnc 'char=?)
		      (= (length elements) 2)
		      (char-ci=? (car elements) (cadr elements)))
		 `(char-ci=? ,sym ,(car elements)))
		
		((and (eq? eqfnc 'string=?)
		      (= (length elements) 2)
		      (string-ci=? (car elements) (cadr elements)))
		 `(string-ci=? ,sym ,(car elements)))
		
		((member elements '((#t #f) (#f #t)))
		 `(boolean? ,sym))		; zero? doesn't happen
		
		(else 
		 `(,func ,sym ',(reverse elements) ,@equals)))))

      (define (reversible-member expr lst)
	(and (pair? lst)
	     (or (member expr lst)
		 (and (eqv? (length expr) 3)
		      (let ((rev-op (hash-table-ref reversibles (car expr))))
			(and rev-op
			     (member (list rev-op (caddr expr) (cadr expr)) lst)))))))

      (define and-rel-ops (let ((h (make-hash-table)))
			    (for-each (lambda (op)
					(hash-table-set! h op #t))
				      '(< = > <= >= char-ci>=? char-ci<? char-ready? char<? char-ci=? char>? 
				        char<=? char-ci>? char-ci<=? char>=? char=? string-ci<=? string=? 
					string-ci>=? string<? string-ci<? string-ci=? string-ci>? string>=? string<=? string>?
					eqv? equal? eq? morally-equal?))
			    h))

      ;; --------------------------------
      ;; start of simplify-boolean code
      ;;   this is not really simplify boolean as in boolean algebra because in scheme there are many unequal truths, but only one falsehood
      ;;   'and and 'or are not boolean operators in a sense

      ;; (format *stderr* "simplify: ~A~%" in-form)
      
      (and (not (or (reversible-member in-form false)
		    (and (pair? in-form)
			 (eq? (car in-form) 'not)
			 (pair? (cdr in-form)) ; (not)!
			 (reversible-member (cadr in-form) true))))
	   (or (and (reversible-member in-form true) #t)
	       (and (pair? in-form)
		    (eq? (car in-form) 'not)
		    (pair? (cdr in-form))
		    (reversible-member (cadr in-form) false) 
		    #t)

      (if (not (pair? in-form))
	  in-form
	  (let ((form (bcomp (bsimp in-form))))

	    (if (not (and (pair? form)
			  (memq (car form) '(or and not))))
		(classify form)
		(let ((len (length form)))
		  (let ((op (case (car form)
			      ((or) 'and)
			      ((and) 'or)
			      (else #f))))
		    (if (and op
			     (>= len 3)
			     (every? (lambda (p) 
				       (and (pair? p)
					    (pair? (cdr p))
					    (pair? (cddr p))
					    (eq? (car p) op)))
				     (cdr form)))
			(let ((first (cadadr form)))
			  (if (every? (lambda (p) 
					(equal? (cadr p) first)) 
				      (cddr form))
			      (set! form `(,op ,first (,(car form) ,@(map (lambda (p)
									    (if (null? (cdddr p))
										(caddr p)
										`(,op ,@(cddr p))))
									  (cdr form)))))
			      (if (null? (cdddr (cadr form)))
				  (let ((last (caddr (cadr form))))
				    (if (every? (lambda (p) 
						  (and (null? (cdddr p))
						       (equal? (caddr p) last)))
						(cddr form))
					(set! form `(,op (,(car form) ,@(map cadr (cdr form))) ,last)))))))))
		  ;; (or (and A B) (and A C)) -> (and A (or B C))
		  ;; (or (and A B) (and C B)) -> (and (or A C) B)
		  ;; (and (or A B) (or A C)) -> (or A (and B C))
		  ;; (and (or A B) (or C B)) -> (or (and A C) B)

		  (case (car form)
		    ;; --------------------------------
		    ((not)

		     (if (not (= len 2))
			 form
			 (let* ((arg (cadr form))
				(val (classify (if (and (pair? arg)
							(memq (car arg) '(and or not)))
						   (simplify-boolean arg true false env)
						   arg)))
				(arg-op (and (pair? arg) 
					     (car arg))))

			   (cond ((boolean? val) 
				  (not val))
				 
				 ((or (code-constant? arg)
				      (and (pair? arg)
					   (symbol? arg-op)
					   (hash-table-ref no-side-effect-functions arg-op)
					   (let ((ret (return-type arg-op env)))
					     (and (or (symbol? ret) (pair? ret))
						  (not (return-type-ok? 'boolean? ret))))
					   (not (var-member arg-op env))))
				  #f)
				 
				 ((and (pair? val)                 ; (not (not ...)) -> ...
				       (pair? (cdr val))           ; this is usually internally generated, 
				       (memq (car val) '(not if cond case begin))) ;   so the message about (and x #t) is in special-case-functions below
				  (case (car val)
				    ((not)
				     (cadr val))

				    ((if)
				     (let ((if-true (simplify-boolean `(not ,(caddr val)) () () env))
					   (if-false (or (not (pair? (cdddr val)))  ; (not #<unspecified>) -> #t
							 (simplify-boolean `(not ,(cadddr val)) () () env))))
				       ;; ideally we'd call if-walker on this to simplify further
				       `(if ,(cadr val) ,if-true ,if-false)))

				    ((cond case)
				     `(,(car val) 
				       ,@(if (eq? (car val) 'cond) () (list (cadr val)))
				       ,@(map (lambda (c)
						(if (not (and (pair? c)
							      (pair? (cdr c))))
						    c
						    (let* ((len (length (cdr c)))
							   (new-last (let ((last (list-ref c len)))
								       (if (and (pair? last)
										(eq? (car last) 'error))
									   last
									   (simplify-boolean `(not ,last) () () env)))))
						      `(,(car c) ,@(copy (cdr c) (make-list (- len 1))) ,new-last))))
					      ((if (eq? (car val) 'cond) cdr cddr) val))))

				    ((begin)
				     (let* ((len1 (- (length val) 1))
					    (new-last (simplify-boolean `(not ,(list-ref val len1)) () () env)))
				       `(,@(copy val (make-list len1)) ,new-last)))))

				 ((not (equal? val arg))
				  `(not ,val))

				 ((not (pair? arg))
				  form)

				 ((and (memq arg-op '(and or))        ; (not (or|and x (not y))) -> (and|or (not x) y)
				       (= (length arg) 3)
				       (or (and (pair? (cadr arg))
						(eq? (caadr arg) 'not))
					   (and (pair? (caddr arg))
						(eq? (caaddr arg) 'not))))
				  (let ((rel (if (eq? arg-op 'or) 'and 'or)))
				    `(,rel ,@(map (lambda (p)
						    (if (and (pair? p)
							     (eq? (car p) 'not))
							(cadr p)
							(simplify-boolean `(not ,p) () () env)))
						  (cdr arg)))))
				 
				 ((<= (length arg) 3)           ; avoid (<= 0 i 12) and such
				  (case arg-op
				    ((< > <= >= odd? even? exact? inexact?char<? char>? char<=? char>=? string<? string>? string<=? string>=?
					char-ci<? char-ci>? char-ci<=? char-ci>=? string-ci<? string-ci>? string-ci<=? string-ci>=?)
				     `(,(hash-table-ref notables arg-op) ,@(cdr arg)))
				    
				    ;; null? is not quite right because (not (null? 3)) -> #t
				    ;; char-upper-case? and lower are not switchable here
				    
				    ((zero?)       ; (not (zero? (logand p 2^n | (ash 1 i)))) -> (logbit? p i)
				     (let ((zarg (cadr arg)))  ; (logand...)
				       (if (not (and (pair? zarg)
						     (eq? (car zarg) 'logand)
						     (pair? (cdr zarg))
						     (pair? (cddr zarg))
						     (null? (cdddr zarg))))
					   form
					   (let ((arg1 (cadr zarg))
						 (arg2 (caddr zarg))) ; these are never reversed
					     (or (and (pair? arg2)
						      (pair? (cdr arg2))
						      (eq? (car arg2) 'ash)
						      (eqv? (cadr arg2) 1)
						      `(logbit? ,arg1 ,(caddr arg2)))
						 (and (integer? arg2)
						      (positive? arg2)
						      (zero? (logand arg2 (- arg2 1))) ; it's a power of 2
						      `(logbit? ,arg1 ,(floor (log arg2 2)))) ; floor for freeBSD?
						 form)))))
				    (else form)))
				 (else form)))))

		    ;; --------------------------------
		    ((or)
		     (case len
		       ((1) #f)
		       ((2) (if (code-constant? (cadr form)) (cadr form) (classify (cadr form))))
		       (else
			(call-with-exit
			 (lambda (return)
			   (when (= len 3)
			     (let ((arg1 (cadr form))
				   (arg2 (caddr form)))

			       (if (and (pair? arg2)     ; (or A (and ... A ...)) -> A
					(eq? (car arg2) 'and)
					(member arg1 (cdr arg2))
					(not (side-effect? arg2 env)))
				   (return arg1))
			       (if (and (pair? arg1)     ; (or (and ... A) A) -> A
					(eq? (car arg1) 'and)
					(equal? arg2 (list-ref arg1 (- (length arg1) 1)))
					(not (side-effect? arg1 env)))
				   (return arg2))

			       (when (pair? arg2)
				 (if (and (eq? (car arg2) 'and) ; (or A (and (not A) B)) -> (or A B)
					  (pair? (cadr arg2))
					  (eq? (caadr arg2) 'not)
					  (equal? arg1 (cadadr arg2)))
				     (return `(or ,arg1 ,@(cddr arg2))))
				 
				 (when (pair? arg1)
				   (when (eq? (car arg1) 'not)
				     (if (symbol? (cadr arg1))
					 (if (memq (cadr arg1) arg2)
					     (begin
					       (if (eq? (car arg2) 'boolean?)
						   (return arg2))
					       (and-incomplete form 'or (cadr arg1) arg2 env))
					     (do ((p arg2 (cdr p)))
						 ((or (not (pair? p))
						      (and (pair? (car p))
							   (memq (cadr arg1) (car p))))
						  (if (pair? p)
						      (and-incomplete form 'or (cadr arg1) (car p) env)))))
					 (if (and (pair? (cadr arg1))   ; (or (not (number? x)) (> x 2)) -> (or (not (real? x)) (> x 2))
						  (hash-table-ref bools (caadr arg1)))
					     (if (member (cadadr arg1) arg2)
						 (and-forgetful form 'or (cadr arg1) arg2 env)
						 (do ((p arg2 (cdr p)))
						     ((or (not (pair? p))
							  (and (pair? (car p))
							       (member (cadadr arg1) (car p))))
						      (if (pair? p)
							  (and-forgetful form 'or (cadr arg1) (car p) env)))))))
				 
				     (if (and (eq? (car arg2) 'and) ; (or (not A) (and A B)) -> (or (not A) B) -- this stuff actually happens!
					      (equal? (cadr arg1) (cadr arg2)))
					 (return `(or ,arg1 ,@(cddr arg2)))))

				   (when (and (eq? (car arg1) 'and)
					      (eq? (car arg2) 'and)
					      (= 3 (length arg1) (length arg2))
					      ;; (not (side-effect? arg1 env)) ; maybe??
					      (or (equal? (cadr arg1) `(not ,(cadr arg2)))
						  (equal? `(not ,(cadr arg1)) (cadr arg2)))
					      (not (equal? (caddr arg1) `(not ,(caddr arg2))))
					      (not (equal? `(not ,(caddr arg1)) (caddr arg2))))
				     ;; kinda dumb, but common: (or (and A B) (and (not A) C)) -> (if A B C)
				     ;;    the other side: (and (or A B) (or (not A) C)) -> (if A C (and B #t)), but it never happens
				     (lint-format "perhaps ~A" 'or 
						  (lists->string form
								 (if (and (pair? (cadr arg1))
									  (eq? (caadr arg1) 'not))
								     `(if ,(cadr arg2) ,(caddr arg2) ,(caddr arg1))
								     `(if ,(cadr arg1) ,(caddr arg1) ,(caddr arg2))))))
				   (let ((t1 (and (pair? (cdr arg1))
						  (pair? (cdr arg2))
						  (or (equal? (cadr arg1) (cadr arg2))
						      (and (pair? (cddr arg2))
							   (null? (cdddr arg2))
							   (equal? (cadr arg1) (caddr arg2))))
						  (not (side-effect? arg1 env))
						  (and-redundant? arg1 arg2))))
				     (if t1
					 (return (if (eq? t1 (car arg1)) arg2 arg1))))

				   ;; if all clauses are (eq-func x y) where one of x/y is a symbol|simple-expr repeated throughout
				   ;;   and the y is a code-constant, or -> memq and friends.  
				   ;;   This could also handle cadr|caddr reversed, but it apparently never happens.
				   (if (and (or (and (eq? (car arg2) '=)
						     (memq (car arg1) '(< > <= >=)))
						(and (eq? (car arg1) '=)
						     (memq (car arg2) '(< > <= >=))))
					    (= (length arg1) 3)
					    (equal? (cdr arg1) (cdr arg2)))
				       (return `(,(if (or (memq (car arg1) '(< <=))
							  (memq (car arg2) '(< <=)))
						      '<= '>=)
						 ,@(cdr arg1))))
				   
				   ;; this makes some of the code above redundant
				   (let ((rel (relsub arg1 arg2 'or env)))
				     (if (or (boolean? rel)
					     (pair? rel))
					 (return rel)))
				   
				   ;; (or (pair? x) (null? x)) -> (list? x)
				   (when (and (pair? (cdr arg1))
					      (pair? (cdr arg2))
					      (equal? (cadr arg1) (cadr arg2)))
				     (if (and (memq (car arg1) '(null? pair?))
					      (memq (car arg2) '(null? pair?))
					      (not (eq? (car arg1) (car arg2))))
					 (return `(list? ,(cadr arg1))))
				     
				     (if (and (eq? (car arg1) 'zero?)  ; (or (zero? x) (positive? x)) -> (not (negative? x)) -- other cases don't happen
					      (memq (car arg2) '(positive? negative?)))
					 (return `(not (,(if (eq? (car arg2) 'positive?) 'negative? 'positive?) ,(cadr arg1))))))
				   
				   ;; (or (and A B) (and (not A) (not B))) -> (eq? (not A) (not B))
				   ;; more accurately (if A B (not B)), but every case I've seen is just boolean
				   ;; perhaps also (or (not (or A B)) (not (or (not A) (not B)))), but it never happens
				   (let ((a1 (cadr form))
					 (a2 (caddr form)))
				     (when (and (pair? a1)
						(pair? a2)
						(eq? (car a1) 'and)
						(eq? (car a2) 'and)
						(= (length a1) 3)
						(= (length a2) 3))
				       (let ((A ((if (and (pair? (cadr a1)) (eq? (caadr a1) 'not)) cadadr cadr) a1))
					     (B (if (and (pair? (caddr a1)) (eq? (caaddr a1) 'not)) (cadr (caddr a1)) (caddr a1))))
					 (if (or (equal? form `(or (and ,A ,B) (and (not ,A) (not ,B))))
						 (equal? form `(or (and (not ,A) (not ,B)) (and ,A ,B))))
					     (return `(eq? (not ,A) (not ,B))))
					 (if (or (equal? form `(or (and ,A (not ,B)) (and (not ,A) ,B)))
						 (equal? form `(or (and (not ,A) ,B) (and ,A (not ,B)))))
					     (return `(not (eq? (not ,A) (not ,B))))))))
				   
				   (when (and (pair? (cdr arg1))
					      (pair? (cdr arg2))
					      (not (eq? (car arg1) (car arg2))))
				     (when (subsumes? (car arg1) (car arg2))
				       (return arg1))
				     
				     (if (eq? (car arg1) 'not)
					 (let ((temp arg1))
					   (set! arg1 arg2)
					   (set! arg2 temp)))
				     (if (and (eq? (car arg2) 'not)
					      (pair? (cadr arg2))
					      (pair? (cdadr arg2))
					      (not (eq? (caadr arg2) 'let?))
					      (or (equal? (cadr arg1) (cadadr arg2))
						  (and (pair? (cddr arg1))
						       (equal? (caddr arg1) (cadadr arg2))))
					      (eq? (return-type (car arg1) env) 'boolean?)
					      (eq? (return-type (caadr arg2) env) 'boolean?))
					 (let ((t2 (or-not-redundant arg1 arg2)))
					   (when t2 
					     (if (eq? t2 'fatuous)
						 (return #t)
						 (if (pair? t2)
						     (return t2)))))))
				   
				   ;; (or (if a c d) (if b c d)) -> (if (or a b) c d) never happens, sad to say
				   ;;   or + if + if does happen but not in this easily optimized form
				   )))) ; len = 3

				;; len > 3 or nothing was caught above
				(let ((nots 0)
				      (revers 0)
				      (arglen (- len 1)))
				  (for-each (lambda (a)
					      (if (pair? a)
						  (if (eq? (car a) 'not)
						      (set! nots (+ nots 1))
						      (if (hash-table-ref notables (car a))
							  (set! revers (+ revers 1))))))
					    (cdr form))
				  (if (= nots arglen)                               ; every arg is `(not ...)
				      (let ((nf (simplify-boolean `(and ,@(map cadr (cdr form))) () () env)))
					(return (simplify-boolean `(not ,nf) () () env)))
				      (if (and (> arglen 2)
					       (or (> nots (/ (* 2 arglen) 3))
						   (and (> arglen 2)
							(> nots (/ arglen 2))
							(> revers 0))))
					  (let ((nf (simplify-boolean `(and ,@(map (lambda (p)
										     (cond ((not (pair? p))
											    `(not ,p))
											   ((eq? (car p) 'not)
											    (cadr p))
											   ((hash-table-ref notables (car p)) => 
											    (lambda (op)
											      `(,op ,@(cdr p))))
											   (else `(not ,p))))
										  (cdr form)))
								      () () env)))
					    (return (simplify-boolean `(not ,nf) () () env))))))

			   (let ((sym #f)
				 (eqfnc #f)
				 (vals ())
				 (start #f))
			     
			     (define (constant-arg p)
			       (if (code-constant? (cadr p))
				   (set! vals (cons (cadr p) vals))
				   (and (code-constant? (caddr p))
					(set! vals (cons (caddr p) vals)))))
			     
			     (define (upgrade-eqf)
			       (set! eqfnc (if (memq eqfnc '(string=? string-ci=? = equal?))
					     'equal?
					     (if (memq eqfnc '(#f eq?)) 'eq? 'eqv?))))
			     
			     (do ((fp (cdr form) (cdr fp)))
				 ((null? fp))
			       (let ((p (and (pair? fp)
					     (car fp))))
				 (if (and (pair? p)
					  (if (not sym)
					      (set! sym (eqv-selector p))
					      (equal? sym (eqv-selector p)))
					  (or (not (memq eqfnc '(char-ci=? string-ci=? =)))
					      (memq (car p) '(char-ci=? string-ci=? =)))
					  
					  ;; = can't share: (equal? 1 1.0) -> #f, so (or (not x) (= x 1)) can't be simplified
					  ;;   except via member+morally-equal? but that brings in float-epsilon and NaN differences.
					  ;;   We could add both: 1 1.0 as in cond?
					  ;;
					  ;; another problem: using memx below means the returned value of the expression
					  ;;   may not match the original (#t -> '(...)), so perhaps we should add a one-time
					  ;;   warning about this, and wrap it in (pair? (mem...)) as an example.
					  ;;
					  ;; and another thing... the original might be broken: (eq? x #(1)) where equal?
					  ;;   is more sensible, but that also changes the behavior of the expression:
					  ;;   (memq x '(#(1))) may be #f (or #t!) when (member x '(#(1))) is '(#(1)).
					  ;;
					  ;; I think I'll try to turn out a more-or-less working expression, but warn about it.
					  
					  (case (car p) 
					    ((string=? equal?)
					     (set! eqfnc (if (or (not eqfnc)
								 (eq? eqfnc (car p)))
							     (car p)
							     'equal?))
					     (and (= (length p) 3)
						  (constant-arg p)))
					    
					    ((char=?)
					     (if (memq eqfnc '(#f char=?))
						 (set! eqfnc 'char=?)
						 (if (not (eq? eqfnc 'equal?))
						     (set! eqfnc 'eqv?)))
					     (and (= (length p) 3)
						  (constant-arg p)))
					    
					    ((eq? eqv?)
					     (let ((leqf (car (->eqf (->lint-type ((if (code-constant? (cadr p)) cadr caddr) p))))))
					       (cond ((not eqfnc) 
						      (set! eqfnc leqf))
						     
						     ((or (memq leqf '(#t equal?))
							  (not (eq? eqfnc leqf)))
						      (set! eqfnc 'equal?))
						     
						     ((memq eqfnc '(#f eq?))
						      (set! eqfnc leqf))))
					     (and (= (length p) 3)
						  (constant-arg p)))
					    
					    ((char-ci=? string-ci=? =)
					     (and (or (not eqfnc)
						      (eq? eqfnc (car p)))
						  (set! eqfnc (car p))
						  (= (length p) 3)
						  (constant-arg p)))
					    
					    ((eof-object?)
					     (upgrade-eqf)
					     (set! vals (cons #<eof> vals)))
					    
					    ((not)
					     (upgrade-eqf)
					     (set! vals (cons #f vals)))
					    
					    ((boolean?) 
					     (upgrade-eqf)
					     (set! vals (cons #f (cons #t vals))))
					    
					    ((zero?)
					     (if (memq eqfnc '(#f eq?)) (set! eqfnc 'eqv?))
					     (set! vals (cons 0 (cons 0.0 vals))))
					    
					    ((null?)
					     (upgrade-eqf)
					     (set! vals (cons () vals)))
					    
					    ((memq memv member)
					     (cond ((eq? (car p) 'member)
						    (set! eqfnc 'equal?))
						   
						   ((eq? (car p) 'memv)
						    (set! eqfnc (if (eq? eqfnc 'string=?) 'equal? 'eqv?)))
						   
						   ((not eqfnc)
						    (set! eqfnc 'eq?)))
					     (and (= (length p) 3)
						  (pair? (caddr p))
						  (eq? 'quote (caaddr p))
						  (pair? (cadr (caddr p)))
						  (set! vals (append (cadr (caddr p)) vals))))
					    
					    (else #f)))
				     (if (not start)
					 (set! start fp)
					 (if (null? (cdr fp))
					    (return (if (eq? start (cdr form))
							(gather-or-eqf-elements eqfnc sym vals)
							`(or ,@(copy (cdr form) (make-list (let loop ((g (cdr form)) (len 0))
											     (if (eq? g start)
												 len
												 (loop (cdr g) (+ len 1))))))
							     ,(gather-or-eqf-elements eqfnc sym vals))))))
				     (when start
				       (if (eq? fp (cdr start))
					   (begin
					     (set! sym #f)
					     (set! eqfnc #f)
					     (set! vals ())
					     (set! start #f))
					   ;; here we have possible header stuff + more than one match + trailing stuff
					   (let ((trailer (if (not (and (pair? fp)
									(pair? (cdr fp))))
							      fp
							      (let ((nfp (simplify-boolean `(or ,@fp) () () env)))
								((if (and (pair? nfp)
									  (eq? (car nfp) 'or))
								     cdr list)
								 nfp)))))
					     (return (if (eq? start (cdr form))
							 `(or ,(gather-or-eqf-elements eqfnc sym vals)
							      ,@trailer)
							 `(or ,@(copy (cdr form) (make-list (let loop ((g (cdr form)) (len 0))
											      (if (eq? g start)
												  len
												  (loop (cdr g) (+ len 1))))))
							      ,(gather-or-eqf-elements eqfnc sym vals)
							      ,@trailer)))))))))
			     
			     (do ((selector #f)              ; (or (and (eq?...)...)....) -> (case ....)
				  (keys ())
				  (fp (cdr form) (cdr fp)))
				 ((or (null? fp)
				      (let ((p (and (pair? fp)
						    (car fp))))
					(not (and (pair? p)
						  (eq? (car p) 'and)
						  (pair? (cdr p))
						  (pair? (cadr p))
						  (pair? (cdadr p))
						  (or selector
						      (set! selector (cadadr p)))
						  (let ((expr (cadr p)))
						    (case (car expr)
						      ((null?)
						       (and (equal? selector (cadr expr))
							    (not (memq () keys))
							    (set! keys (cons () keys))))
						      ;; we have to make sure no keys are repeated:
						      ;;   (or (and (eq? x 'a) (< y 1)) (and (eq? x 'a) (< y 2)))
						      ;;   this rewrite has become much trickier than expected...
						      
						      ((boolean?)
						       (and (equal? selector (cadr expr))
							    (not (memq #f keys))
							    (not (memq #t keys))
							    (set! keys (cons #f (cons #t keys)))))
						      
						      ((eof-object?)
						       (and (equal? selector (cadr expr))
							    (not (memq #<eof> keys))
							    (set! keys (cons #<eof> keys))))
						      
						      ((zero?)
						       (and (equal? selector (cadr expr))
							    (not (memv 0 keys))
							    (not (memv 0.0 keys))
							    (set! keys (cons 0.0 (cons 0 keys)))))
						      
						      ((memq memv)
						       (and (equal? selector (cadr expr))
							    (pair? (cddr expr))
							    (pair? (caddr expr))
							    (eq? (caaddr expr) 'quote)
							    (pair? (cadr (caddr expr)))
							    (not (any? (lambda (g)
									 (memv g keys))
								       (cadr (caddr expr))))
							    (set! keys (append (cadr (caddr expr)) keys))))
						      
						      ((eq? eqv? char=?)
						       (and (pair? (cddr expr))
							    (null? (cdddr expr))
							    (or (and (equal? selector (cadr expr))
								     (code-constant? (caddr expr))
								     (not (memv (unquoted (caddr expr)) keys))
								     (set! keys (cons (unquoted (caddr expr)) keys)))
								(and (equal? selector (caddr expr))
								     (code-constant? (cadr expr))
								     (not (memv (unquoted (cadr expr)) keys))
								     (set! keys (cons (unquoted (cadr expr)) keys))))))
						      
						      ((not)
						       ;; no hits here for last+not eq(etc)+no collision in keys
						       (and (equal? selector (cadr expr))
							    (not (memq #f keys))
							    (set! keys (cons #f keys))))
						      
						      (else #f)))))))
				  (if (null? fp)
				      (return `(case ,selector
						 ,@(map (lambda (p)
							  (let ((result (if (null? (cdddr p))
									    (caddr p)
									    `(and ,@(cddr p))))
								(key (let ((expr (cadr p)))
								       (case (car expr)
									 ((eq? eqv? char=?)
									  (if (equal? selector (cadr expr))
									      (list (unquoted (caddr expr)))
									      (list (unquoted (cadr expr)))))
									 ((memq memv)   (unquoted (caddr expr)))
									 ((null?)       (list ()))
									 ((eof-object?) (list #<eof>))
									 ((zero?)       (list 0 0.0))
									 ((not)         (list #f))
									 ((boolean?)    (list #t #f))))))
							    (list key result)))
							(cdr form))
						 (else #f))))))
			     
			     (do ((new-form ())
				  (retry #f)
				  (exprs (cdr form) (cdr exprs)))
				 ((null? exprs) 
				  (return (and (pair? new-form)
					       (if (null? (cdr new-form))
						   (car new-form)
						   (if retry
						       (simplify-boolean `(or ,@(reverse new-form)) () () env)
						       `(or ,@(reverse new-form)))))))
			       (let ((val (classify (car exprs)))
				     (old-form new-form))
				 
				 (when (and (pair? val)
					    (memq (car val) '(and or not)))
				   (set! val (classify (simplify-boolean val true false env)))
				   (when (and (> len 3)
					      (pair? val)
					      (eq? (car val) 'not)
					      (pair? (cdr exprs)))
				     (if (symbol? (cadr val))
					 (if (and (pair? (cadr exprs))
						  (memq (cadr val) (cadr exprs)))
					     (and-incomplete form 'or (cadr val) (cadr exprs) env)
					     (do ((ip (cdr exprs) (cdr ip))
						  (found-it #f))
						 ((or found-it
						      (not (pair? ip))))
					       (do ((p (car ip) (cdr p)))
						   ((or (not (pair? p))
							(and (memq (cadr val) p)
							     (set! found-it p)))
						    (if (pair? found-it)
							(and-incomplete form 'or (cadr val) found-it env))))))
					 (when (and (pair? (cadr val))
						    (pair? (cadr exprs))
						    (hash-table-ref bools (caadr val)))
					   (if (member (cadadr val) (cadr exprs))
					       (and-forgetful form 'or (cadr val) (cadr exprs) env)
					       (do ((p (cadr exprs) (cdr p)))
						   ((or (not (pair? p))
							(and (pair? (car p))
							     (member (cadadr val) (car p))))
						    (if (pair? p)
							(and-forgetful form 'or (cadr val) (car p) env)))))))))
				 (if (not (or retry
					      (equal? val (car exprs))))
				     (set! retry #t))
				 
				 (if val                                   ; #f in or is ignored
				     (cond ((or (eq? val #t)               ; #t or any non-#f constant in or ends the expression
						(code-constant? val))
					    (set! new-form (if (null? new-form) ; (or x1 123) -> value of x1 first
							       (list val)
							       (cons val new-form)))
					    ;; reversed when returned
					    (set! exprs '(#t)))
					   
					   ((and (pair? val)                       ; (or ...) -> splice into current
						 (eq? (car val) 'or))
					    (set! exprs (append val (cdr exprs)))) ; we'll skip the 'or in do step
					   
					   ((not (or (memq val new-form)
						     (and (pair? val)         ;   and redundant tests
							  (hash-table-ref booleans (car val))
							  (any? (lambda (p)
								  (and (pair? p)
								       (subsumes? (car p) (car val))
								       (equal? (cadr val) (cadr p))))
								new-form))))
					    (set! new-form (cons val new-form)))))
				 
				 (if (and (not (eq? new-form old-form))
					  (pair? (cdr new-form)))
				     (let ((rel (relsub (cadr new-form) (car new-form) 'or env))) ; new-form is reversed
				       (if (or (boolean? rel)
					       (pair? rel))
					   (set! new-form (cons rel (cddr new-form))))))))))))))

		    ;; --------------------------------
		    ((and)
		     (case len
		       ((1) #t)
		       ((2) (classify (cadr form)))
		       (else
			(and (not (contradictory? (cdr form)))
			     (call-with-exit
			      (lambda (return)
				(when (= len 3)
				  (let ((arg1 (cadr form))
					(arg2 (caddr form)))
				    (if (and (pair? arg2)                ; (and A (or A ...)) -> A
					     (eq? (car arg2) 'or)
					     (equal? arg1 (cadr arg2))
					     (not (side-effect? arg2 env)))
					(return arg1))
				    (if (and (pair? arg1)                ; (and (or ... A ...) A) -> A
					     (eq? (car arg1) 'or)
					     (member arg2 (cdr arg1))
					     (not (side-effect? arg1 env)))
					(return arg2))
				    ;; the and equivalent of (or (not A) (and A B)) never happens

				    (when (pair? arg2)
				      (if (symbol? arg1)                 ; (and x (pair? x)) -> (pair? x)
					  (if (memq arg1 arg2)
					      (begin
						(case (car arg2) 
						  ((not)      (return #f))
						  ((boolean?) (return `(eq? ,arg1 #t))))
						(and-incomplete form 'and arg1 arg2 env)
						(if (hash-table-ref booleans (car arg2))
						    (return arg2)))
					      (do ((p arg2 (cdr p)))   ; (and x (+ (log x) 1)) -> (and (number? x)...)
						  ((or (not (pair? p))
						       (and (pair? (car p))
							    (memq arg1 (car p))))
						   (if (pair? p)
						       (and-incomplete form 'and arg1 (car p) env)))))
					  (if (and (pair? arg1)               ; (and (number? x) (> x 2)) -> (and (real? x) (> x 2))
						   (hash-table-ref bools (car arg1)))
					      (if (member (cadr arg1) arg2)
						  (and-forgetful form 'and arg1 arg2 env)
						  (do ((p arg2 (cdr p)))   
						      ((or (not (pair? p))
							   (and (pair? (car p))
								(member (cadr arg1) (car p))))
						       (if (pair? p)
							   (and-forgetful form 'and arg1 (car p) env))))))))
				    
				    (if (and (not (side-effect? arg1 env))
					     (equal? arg1 arg2))                  ; (and x x) -> x
					(return arg1))
				    
				    (when (and (pair? arg1)
					       (pair? arg2)
					       (pair? (cdr arg1))
					       (pair? (cdr arg2)))

				      (let ((t1 (and (or (equal? (cadr arg1) (cadr arg2))
							 (and (pair? (cddr arg2))
							      (null? (cdddr arg2))
							      (equal? (cadr arg1) (caddr arg2))))
						     (not (side-effect? arg1 env))
						     (and-redundant? arg1 arg2)))) ; (and (integer? x) (number? x)) -> (integer? x)
					(if t1
					    (return (cond 
						     ((memq t1 '(eq? eqv? equal?))
						      `(,t1 ,@(cdr arg2)))
						     
						     ((eq? t1 'memv)
						      (let ((x ((if (equal? (cadr arg1) (cadr arg2)) caddr cadr) arg2)))
							(if (rational? x)
							    `(memv ,(cadr arg1) '(,x ,(* 1.0 x)))
							    `(memv ,(cadr arg1) '(,(floor x) ,x)))))
						     
						     ((eq? t1 (car arg1)) arg1)
						     (else arg2)))))

				      (when (and (hash-table-ref reversibles (car arg1))
						 (pair? (cddr arg1))
						 (null? (cdddr arg1))
						 (pair? (cddr arg2))
						 (null? (cdddr arg2))
						 (not (side-effect? arg2 env))             ; arg1 is hit in any case
						 (or (eq? (car arg1) (car arg2))           ; either ops are equal or
						     (let ((rf (hash-table-ref reversibles (car arg2))))  ;    try reversed op for arg2
						       (and (eq? (car arg1) rf)
							    (set! arg2 (cons rf (reverse (cdr arg2))))))))
					(when (and (memq (car arg1) '(< <= >= >))       ; (and (op x y) (op x z)) -> (op x (min|max y z))
						   (equal? (cadr arg1) (cadr arg2)))
					  (if (and (rational? (caddr arg1))
						   (rational? (caddr arg2)))
					      (return `(,(car arg1) 
							,(cadr arg1)
							,((if (memq (car arg1) '(< <=)) min max) (caddr arg1) (caddr arg2)))))
					  (return `(,(car arg1) 
						    ,(cadr arg1)
						    (,(if (memq (car arg1) '(< <=)) 'min 'max) ,(caddr arg1) ,(caddr arg2)))))

					(when (or (equal? (caddr arg1) (cadr arg2))     ; (and (op x y) (op y z))
						  (equal? (cadr arg1) (caddr arg2))     ; (and (op x y) (op z x))
						  (and (memq (car arg1) '(= char=? string=? char-ci=? string-ci=?))
						       (or (equal? (cadr arg1) (cadr arg2))
							   (equal? (caddr arg1) (caddr arg2)))))
					  (let ((op1 (car arg1))
						(arg1-1 (cadr arg1))
						(arg1-2 (caddr arg1))
						(arg2-1 (cadr arg2))
						(arg2-2 (caddr arg2)))
					    (return
					     (cond ((equal? arg1-2 arg2-1)       ; (and (op x y) (op y z)) -> (op x y z)
						    (if (equal? arg1-1 arg2-2)
							(if (memq op1 '(= char=? string=? char-ci=? string-ci=?))
							    arg1 
							    (and (memq op1 '(<= >= char<=? char>=? string<=? string>=?
										char-ci<=? char-ci>=? string-ci<=? string-ci>=?))
								 `(,(case op1 
								      ((>= <=) '=)
								      ((char<= char>=) 'char=?)
								      ((char-ci<= char-ci>=) 'char-ci=?)
								      ((string<= string>=) 'string=?)
								      ((string-ci<= string-ci>=) 'string-ci=?))
								   ,@(cdr arg1))))
							(and (or (not (code-constant? arg1-1))
								 (not (code-constant? arg2-2))
								 ((symbol->value op1) arg1-1 arg2-2))
							     `(,op1 ,arg1-1 ,arg2-1 ,arg2-2))))
						   
						   ((equal? arg1-1 arg2-2)       ; (and (op x y) (op z x)) -> (op z x y)
						    (if (equal? arg1-2 arg2-1)
							(and (memq op1 '(= char=? string=? char-ci=? string-ci=?))
							     arg1)
							(and (or (not (code-constant? arg2-1))
								 (not (code-constant? arg1-2))
								 ((symbol->value op1) arg2-1 arg1-2))
							     `(,op1 ,arg2-1 ,arg1-1 ,arg1-2))))
						   
						   ;; here we're restricted to equalities and we know arg1 != arg2
						   ((equal? arg1-1 arg2-1)        ; (and (op x y) (op x z)) -> (op x y z)
						    (if (and (code-constant? arg1-2)
							     (code-constant? arg2-2))
							(and ((symbol->value op1) arg1-2 arg2-2)
							     arg1)
							`(,op1 ,arg1-1 ,arg1-2 ,arg2-2)))
						   
						   ;; equalities again
						   ((and (code-constant? arg1-1)
							 (code-constant? arg2-1))
						    (and ((symbol->value op1) arg1-1 arg2-1)
							 arg1))
						   
						   (else `(,op1 ,arg1-1 ,arg1-2 ,arg2-1)))))))
				      
				      ;; check some special cases 
				      (when (and (or (equal? (cadr arg1) (cadr arg2))
						     (and (pair? (cddr arg2))
							  (null? (cdddr arg2))
							  (equal? (cadr arg1) (caddr arg2))))
						 (hash-table-ref booleans (car arg1)))
					
					(when (or (eq? (car arg1) 'zero?)  ; perhaps rational? and integer? here -- not many hits
						  (eq? (car arg2) 'zero?))
					  (if (or (memq (car arg1) '(integer? rational? exact?))
						  (memq (car arg2) '(integer? rational? exact?)))
					      (return `(eqv? ,(cadr arg1) 0)))
					  (if (or (eq? (car arg1) 'inexact?)
						  (eq? (car arg2) 'inexact?))
					      (return `(eqv? ,(cadr arg1) 0.0))))
					
					(when (hash-table-ref and-rel-ops (car arg2))
					  (when (and (eq? (car arg1) 'symbol?)
						     (memq (car arg2) '(eq? eqv? equal?))
						     (or (quoted-symbol? (cadr arg2))
							 (quoted-symbol? (caddr arg2))))
					    (return `(eq? ,@(cdr arg2))))
					  
					  (when (and (eq? (car arg1) 'positive?)
						     (eq? (car arg2) '<)
						     (eq? (cadr arg1) (cadr arg2)))
					    (return `(< 0 ,(cadr arg1) ,(caddr arg2))))))

				      (when (and (member (cadr arg1) arg2)
						 (memq (car arg2) '(string=? char=? eq? eqv? equal?))
						 (null? (cdddr arg2))
						 (hash-table-ref bools (car arg1))
						 (or (and (code-constant? (cadr arg2))
							  (compatible? (car arg1) (->lint-type (cadr arg2))))
						     (and (code-constant? (caddr arg2))
							  (compatible? (car arg1) (->lint-type (caddr arg2))))))
					(return `(,(if (eq? (car arg1) 'char?) ,eqv? 'equal?) ,@(cdr arg2))))

				      (when (and (equal? (cadr arg1) (cadr arg2))
						 (eq? (car arg1) 'inexact?)
						 (eq? (car arg2) 'real?))
					(return `(and ,arg2 ,arg1)))

				      ;; this makes some of the code above redundant
				      (let ((rel (relsub arg1 arg2 'and env)))
					(if (or (boolean? rel)
						(pair? rel))
					    (return rel)))
				      
				      ;; (and ... (not...))
				      (unless (eq? (car arg1) (car arg2))
					(if (eq? (car arg1) 'not)
					    (let ((temp arg1))
					      (set! arg1 arg2)
					      (set! arg2 temp)))
					(when (and (eq? (car arg2) 'not)
						   (pair? (cadr arg2))
						   (pair? (cdadr arg2))
						   (not (eq? (caadr arg2) 'let?))
						   (or (equal? (cadr arg1) (cadadr arg2))
						       (and (pair? (cddr arg1))
							    (equal? (caddr arg1) (cadadr arg2))))
						   (eq? (return-type (car arg1) env) 'boolean?)
						   (eq? (return-type (caadr arg2) env) 'boolean?))
					  (let ((t2 (and-not-redundant arg1 arg2)))
					    (cond ;((not t2) #f)
						  ((eq? t2 'contradictory) (return #f))
						  ((symbol? t2) (return `(,t2 ,@(cdr arg1))))
						  ((pair? t2)	(return t2))))))
				      
				      (if (hash-table-ref bools (car arg1))
					  (let ((p (member (cadr arg1) (cdr arg2))))
					    (when p
					      (let ((sig (arg-signature (car arg2) env))
						    (pos (- (length arg2) (length p))))
						(when (pair? sig)
						  (let ((arg-type (and (> (length sig) pos)
								       (list-ref sig pos))))
						    (unless (compatible? (car arg1) arg-type)
						      (let ((ln (and (< 0 line-number 100000) line-number)))
							(format outport "~NCin ~A~A, ~A is ~A, but ~A wants ~A"
								lint-left-margin #\space 
								(truncated-list->string form) 
								(if ln (format #f " (line ~D)" ln) "")
								(cadr arg1) 
								(prettify-checker-unq (car arg1))
								(car arg2)
								(prettify-checker arg-type))))))))))

				      (cond ((not (and (eq? (car arg1) 'equal?) ; (and (equal? (car a1) (car a2)) (equal? (cdr a1) (cdr a2))) -> (equal? a1 a2)
						       (eq? (car arg2) 'equal?)
						       (pair? (cadr arg1))
						       (pair? (caddr arg1))
						       (pair? (cadr arg2))
						       (pair? (caddr arg2))
						       (eq? (caadr arg1) (caaddr arg1)))))

					    ((assq (caadr arg1)
						   '((car cdr #t) 
						     (caar cdar car) (cadr cddr cdr)
						     (caaar cdaar caar) (caadr cdadr cadr) (caddr cdddr cddr) (cadar cddar cdar)
						     (cadddr cddddr cdddr) (caaaar cdaaar caaar) (caaadr cdaadr caadr) (caadar cdadar cadar)
						     (caaddr cdaddr caddr) (cadaar cddaar cdaar) (cadadr cddadr cdadr) (caddar cdddar cddar)))
					     => (lambda (x)
						  (if (and (eq? (caadr arg2) (cadr x))
							   (eq? (caaddr arg2) (cadr x))
							   (equal? (cadadr arg1) (cadadr arg2))
							   (equal? (cadr (caddr arg1)) (cadr (caddr arg2))))
						      (return (if (symbol? (caddr x))
								  `(equal? (,(caddr x) ,(cadadr arg1)) (,(caddr x) ,(cadr (caddr arg1))))
								  `(equal? ,(cadadr arg1) ,(cadr (caddr arg1)))))))))
				      )))

				;; len > 3 or nothing was caught above
				(let ((nots 0)
				      (revers 0)
				      (arglen (- len 1)))
				  (for-each (lambda (a)
					      (if (pair? a)
						  (if (eq? (car a) 'not)
						      (set! nots (+ nots 1))
						      (if (hash-table-ref notables (car a))
							  (set! revers (+ revers 1))))))
					    (cdr form))
				  (if (= nots arglen)                               ; every arg is `(not ...)
				      (let ((nf (simplify-boolean `(or ,@(map cadr (cdr form))) () () env)))
					(return (simplify-boolean `(not ,nf) () () env)))
				      (if (and (> arglen 2)
					       (or (>= nots (/ (* 3 arglen) 4))      ; > 2/3 seems to get some ugly rewrites
						   (and (>= nots (/ (* 2 arglen) 3)) ; was > 1/2 here
							(> revers 0))))
					  (let ((nf (simplify-boolean `(or ,@(map (lambda (p)
										    (cond ((not (pair? p))
											   `(not ,p))
											  ((eq? (car p) 'not)
											   (cadr p))
											  ((hash-table-ref notables (car p)) => 
											   (lambda (op)
											     `(,op ,@(cdr p))))
											  (else `(not ,p))))
										  (cdr form)))
								      () () env)))
					    (return (simplify-boolean `(not ,nf) () () env))))))
				
				(if (every? (lambda (a)
					      (and (pair? a)
						   (eq? (car a) 'zero?)))
					    (cdr form))
				    (return `(= 0 ,@(map cadr (cdr form)))))
				
				(let ((diff (apply and-redundants env (cdr form))))
				  (when diff 
				    (if (null? (cdr diff))
					(return (car diff)))
				    (return (simplify-boolean `(and ,@diff) () () env))))
				;; now there are redundancies below (see subsumes?) but they assumed the tests were side-by-side

				(do ((new-form ())
				     (retry #f)
				     (exprs (cdr form) (cdr exprs)))
				    ((null? exprs) 
				     (or (null? new-form)      ; (and) -> #t
					 (let ((newer-form (let ((nform (reverse new-form)))
							     (map (lambda (x cdr-x)
								    (if (and x (code-constant? x))
									(values)
									x))
								  nform (cdr nform)))))
					   (return
					    (cond ((null? newer-form)
						   (car new-form))
						  
						  ((and (eq? (car new-form) #t) ; trailing #t is dumb if next-to-last is boolean func
							(pair? (cdr new-form))
							(pair? (cadr new-form))
							(symbol? (caadr new-form))
							(eq? (return-type (caadr new-form) env) 'boolean?))
						   (if (null? (cdr newer-form))
						       (car newer-form)
						       `(and ,@newer-form)))
						  
						  (retry
						   (simplify-boolean `(and ,@newer-form ,(car new-form)) () () env))
						  
						  (else `(and ,@newer-form ,(car new-form))))))))
				  
				  (let* ((e (car exprs))
					 (val (classify e))
					 (old-form new-form))

				    (if (and (pair? val)
					     (memq (car val) '(and or not)))
					(set! val (classify (set! e (simplify-boolean val () false env))))
					
					(when (and (> len 3)
						   (pair? (cdr exprs)))
					  (if (symbol? val)
					      (if (and (pair? (cadr exprs))
						       (memq val (cadr exprs)))
						  (let ((nval (simplify-boolean `(and ,val ,(cadr exprs)) () false env)))
						    (if (and (pair? nval)
							     (eq? (car nval) 'and))
							(and-incomplete form 'and val (cadr exprs) env)
							(begin
							  (set! val nval)
							  (set! exprs (cdr exprs)))))
						  (do ((ip (cdr exprs) (cdr ip))
						       (found-it #f))
						      ((or found-it
							   (not (pair? ip))))
						    (do ((p (car ip) (cdr p)))
							((or (not (pair? p))
							     (and (memq val p)
								  (let ((nval (simplify-boolean `(and ,val ,p) () false env)))
								    (if (and (pair? nval)
									     (eq? (car nval) 'and))
									(set! found-it p)
									(let ((ln (and (< 0 line-number 100000) line-number)))
									  (format outport "~NCin ~A~A,~%~NCperhaps change ~S to ~S~%"
										  lint-left-margin #\space 
										  (truncated-list->string form)
										  (if ln (format #f " (line ~D)" ln) "")
										  (+ lint-left-margin 4) #\space 
										  `(and ... ,val ... ,p) 
										  nval)
									  (set! found-it #t)))))
							     (and (pair? (car p))
								  (memq val (car p))
								  (set! found-it (car p))))
							 (if (pair? found-it)
							     (and-incomplete form 'and val found-it env))))))
					      (when (and (pair? val) 
							 (pair? (cadr exprs))
							 (hash-table-ref bools (car val)))
						(if (member (cadr val) (cadr exprs))
						    (and-forgetful form 'and val (cadr exprs) env)
						    (do ((p (cadr exprs) (cdr p)))
							((or (not (pair? p))
							     (and (pair? (car p))
								  (member (cadr val) (car p))))
							 (if (pair? p)
							     (and-forgetful form 'and val (car p) env)))))))))
				    
				    (if (not (or retry
						 (equal? e (car exprs))))
					(set! retry #t))

				    ;(format *stderr* "val: ~A, e: ~A~%" val e)
				    
				    ;; (and x1 x2 x1) is not reducible
				    ;;   the final thing has to remain at the end, but can be deleted earlier if it can't short-circuit the evaluation,
				    ;;   but if there are expressions following the first x1, we can't be sure that it is not
				    ;;   protecting them:
				    ;;       (and false-or-0 (display (list-ref lst false-or-0)) false-or-0)
				    ;;   so I'll not try to optimize that case.  But (and x x) is optimizable.
				    
				    (cond ((eq? val #t)
					   (if (null? (cdr exprs))     ; (and x y #t) should not remove the #t
					       (if (or (and (pair? e)
							    (eq? (return-type (car e) env) 'boolean?))
						       (eq? e #t))
						   (set! new-form (cons val new-form))
						   (if (or (null? new-form)
							   (not (equal? e (car new-form))))
						       (set! new-form (cons e new-form))))
					       (if (and (not (eq? e #t))
							(or (null? new-form)
							    (not (member e new-form))))
						   (set! new-form (cons e new-form)))))
					  
					  ((not val)             ; #f in 'and' ends the expression
					   (set! new-form (if (or (null? new-form)   
								  (just-symbols? new-form))
							      '(#f)
							      (cons #f new-form)))
					   (set! exprs '(#f)))
					  
					  ((and (pair? e)       ; if (and ...) splice into current
						(eq? (car e) 'and))
					   (set! exprs (append e (cdr exprs))))
					  
					  ((and (pair? e)       ; (and (list? p) (pair? p) ...) -> (and (pair? p) ...)
						(pair? (cdr exprs))
						(pair? (cadr exprs))
						(eq? (and-redundant? e (cadr exprs)) (caadr exprs))
						(equal? (cadr e) (cadadr exprs))))
					  
					  ((and (pair? e)       ; (and (list? p) (not (null? p)) ...) -> (and (pair? p) ...)
						(memq (car e) '(list? pair?))
						(pair? (cdr exprs))
						(let ((p (cadr exprs)))
						  (and (pair? p)
						       (eq? (car p) 'not)
						       (pair? (cadr p))
						       (eq? (caadr p) 'null?)
						       (equal? (cadr e) (cadadr p)))))
					   (set! new-form (cons `(pair? ,(cadr e)) new-form))
					   (set! exprs (cdr exprs)))
					  
					  ((not (and (pair? e)                   ; (and ... (or ... 123) ...) -> splice out or
						     (pair? (cdr exprs))
						     (eq? (car e) 'or)
						     (pair? (cdr e))
						     (pair? (cddr e))
						     (cond ((list-ref e (- (length e) 1)) => code-constant?) ; (or ... #f)
							   (else #f))))
					   (if (not (and (pair? new-form)
							 (or (eq? val (car new-form)) ; omit repeated tests
							     (and (pair? val)         ;   and redundant tests
								  (hash-table-ref booleans (car val))
								  (any? (lambda (p)
									  (and (pair? p)
									       (subsumes? (car val) (car p))
									       (equal? (cadr val) (cadr p))))
									new-form)))))
					       (set! new-form (cons val new-form)))))
				    
				    (if (and (not (eq? new-form old-form))
					     (pair? (cdr new-form)))
					(let ((rel (relsub (car new-form) (cadr new-form) 'and env)))
					  ;; rel #f should halt everything as above, and it looks ugly in the output,
					  ;;   but it never happens in real code
					  (if (or (pair? rel)
						  (boolean? rel))
					      (set! new-form (cons rel (cddr new-form))))))))))))))))))))))))
    
    (define (splice-if f lst)
      (cond ((null? lst) ())
	    ((not (pair? lst)) lst)
	    ((and (pair? (car lst)) 
		  (f (caar lst)))
	     (append (splice-if f (cdar lst)) 
		     (splice-if f (cdr lst))))
	    (else (cons (car lst) 
			(splice-if f (cdr lst))))))

    (define (horners-rule form)
      (and (pair? form)
	   (call-with-exit 
	    (lambda (return)
	      (do ((p form (cdr p))
		   (coeffs #f)
		   (top 0)
		   (sym #f))
		  ((not (pair? p))
		   (do ((x (- top 1) (- x 1))
			(result (coeffs top)))
		       ((< x 0)
			result)
		     (set! result 
			   (if (zero? (coeffs x))
			       `(* ,sym ,result)
			       `(+ ,(coeffs x) (* ,sym ,result))))))
		(let ((cx (car p)))
		  (cond ((number? cx)
			 (if (not coeffs) (set! coeffs (make-vector 4 0)))
			 (set! (coeffs 0) (+ (coeffs 0) cx)))

			((symbol? cx)
			 (if (not sym)
			     (set! sym cx)
			     (if (not (eq? sym cx))
				 (return #f)))
			 (if (not coeffs) (set! coeffs (make-vector 4 0)))
			 (set! top (max top 1))
			 (set! (coeffs 1) (+ (coeffs 1) 1)))

			((not (and (pair? cx)
				   (eq? (car cx) '*)))
			 (return #f))

			(else
			 (let ((ctr 0)
			       (ax 1))
			   (for-each (lambda (qx)
				       (if (symbol? qx)
					   (if (not sym)
					       (begin
						 (set! sym qx)
						 (set! ctr 1))
					       (if (not (eq? sym qx))
						   (return #f)
						   (set! ctr (+ ctr 1))))
					   (if (number? qx)
					       (set! ax (* ax qx))
					       (return #f))))
				     (cdr cx))
			   (if (not coeffs) (set! coeffs (make-vector 4 0)))
			   (if (>= ctr (length coeffs))
			       (set! coeffs (copy coeffs (make-vector (* ctr 2) 0))))
			   (set! top (max top ctr))
			   (set! (coeffs ctr) (+ (coeffs ctr) ax)))))))))))
    
    (define (simplify-numerics form env)
      ;; this returns a form, possibly the original simplified
      (let ((real-result? (lambda (op) (memq op '(imag-part real-part abs magnitude angle max min exact->inexact inexact
						  modulo remainder quotient lcm gcd))))
	    (rational-result? (lambda (op) (memq op '(rationalize inexact->exact exact))))
	    (integer-result? (lambda (op) (memq op '(logior lognot logxor logand numerator denominator floor round truncate ceiling ash)))))
	
	(define (inverse-op op)
	  (case op 
	    ((sin) 'asin) ((cos) 'acos) ((tan) 'atan) ((asin) 'sin) ((acos) 'cos) ((atan) 'tan)
	    ((sinh) 'asinh) ((cosh) 'acosh) ((tanh) 'atanh) ((asinh) 'sinh) ((acosh) 'cosh) ((atanh) 'tanh)
	    ((log) 'exp) ((exp) 'log)))
	
	(define (just-rationals? form)
	  (or (null? form)
	      (rational? form)
	      (and (pair? form)
		   (rational? (car form))
		   (just-rationals? (cdr form)))))
	
	(define (just-reals? form)
	  (or (null? form)
	      (real? form)
	      (and (pair? form)
		   (real? (car form))
		   (just-reals? (cdr form)))))
	
	(define (just-integers? form)
	  (or (null? form)
	      (integer? form)
	      (and (pair? form)
		   (integer? (car form))
		   (just-integers? (cdr form)))))
	
	(define (simplify-arg x)
	  (if (or (null? x)                      ; constants and the like look dumb if simplified
		  (not (proper-list? x))
		  (not (hash-table-ref no-side-effect-functions (car x)))
		  (var-member (car x) env))
	      x
	      (let ((f (simplify-numerics x env)))
		(if (and (pair? f)
			 (just-rationals? f))
		    (catch #t
		      (lambda ()
			(eval f))
		      (lambda ignore f))
		    f))))
	
	(define (remove-inexactions val)
	  (when (and (or (assq 'exact->inexact val)
			 (assq 'inexact val))
		     (not (tree-member 'random val))
		     (any? number? val))
	    (set! val (map (lambda (x)
			     (if (and (pair? x)
				      (memq (car x) '(inexact exact->inexact)))
				 (cadr x)
				 x))
			   val))
	    (if (not (any? (lambda (x)
			     (and (number? x)
				  (inexact? x)))
			   val))
		(do ((p val (cdr p)))
		    ((or (null? p)
			 (number? (car p)))
		     (if (pair? p)
			 (set-car! p (* 1.0 (car p))))))))
	  val)

	(let* ((args (map simplify-arg (cdr form)))
	       (len (length args)))
	  (case (car form)

	    ((+)
	     (case len
	       ((0) 0)
	       ((1) (car args))
	       (else 
		(let ((val (remove-all 0 (splice-if (lambda (x) (eq? x '+)) args))))
		  (if (every? (lambda (x) (or (not (number? x)) (rational? x))) val)
		      (let ((rats (collect-if list rational? val)))
			(if (and (pair? rats)
				 (pair? (cdr rats)))
			    (let ((y (apply + rats)))
			      (set! val (if (zero? y) 
					    (collect-if list (lambda (x) (not (number? x))) val)
					    (cons y (collect-if list (lambda (x) (not (number? x))) val))))))))
		  (set! val (remove-inexactions val))
		  (if (any? (lambda (p)        ; collect all + and - vals -> (- (+ ...) ...)
			      (and (pair? p) 
				   (eq? (car p) '-)))
			    val)
		      (let ((plus ())
			    (minus ())
			    (c 0))
			(for-each (lambda (p)
				    (if (not (and (pair? p)
						  (eq? (car p) '-)))
					(if (rational? p)
					    (set! c (+ c p))
					    (set! plus (cons p plus)))
					(if (null? (cddr p))
					    (if (rational? (cadr p))
						(set! c (- c (cadr p)))
						(set! minus (cons (cadr p) minus)))
					    (begin
					      (if (rational? (cadr p))
						  (set! c (+ c (cadr p)))
						  (set! plus (cons (cadr p) plus)))
					      (for-each (lambda (p1) 
							  (if (rational? p1) 
							      (set! c (- c p1)) 
							      (set! minus (cons p1 minus))))
							(cddr p))))))
				  val)
			(simplify-numerics `(- (+ ,@(reverse plus) ,@(if (positive? c) (list c) ()))
					       ,@(reverse minus) ,@(if (negative? c) (list (abs c)) ()))
					   env))
		      
		      (case (length val)
			((0) 0)
			((1) (car val))                              ; (+ x) -> x
			((2)
			 (let ((arg1 (car val))
			       (arg2 (cadr val)))
			   (cond ((and (real? arg2)                  ; (+ x -1) -> (- x 1)
				       (negative? arg2)
				       (not (number? arg1)))
				  `(- ,arg1 ,(abs arg2)))
				 
				 ((and (real? arg1)                  ; (+ -1 x) -> (- x 1)
				       (negative? arg1)
				       (not (number? arg2)))
				  `(- ,arg2 ,(abs arg1)))
				 
				 ((and (pair? arg1)
				       (eq? (car arg1) '*)           ; (+ (* a b) (* a c)) -> (* a (+ b c))
				       (pair? arg2)
				       (eq? (car arg2) '*)
				       (any? (lambda (a)
					       (member a (cdr arg2)))
					     (cdr arg1)))
				  (do ((times ())
				       (pluses ())
				       (rset (cdr arg2))
				       (p (cdr arg1) (cdr p)))
				      ((null? p)
				       ;; times won't be () because we checked above for a match
				       ;;  if pluses is (), arg1 is completely included in arg2
				       ;;  if rset is (), arg2 is included in arg1
				       (simplify-numerics `(* ,@(reverse times)
							      (+ (* ,@(reverse (if (pair? pluses) pluses (list (if (null? pluses) 1 pluses)))))
								 (* ,@rset)))
							  env))
				    (if (member (car p) rset)
					(begin
					  (set! times (cons (car p) times))
					  (set! rset (remove (car p) rset)))
					(set! pluses (cons (car p) pluses)))))
				 
				 ((and (pair? arg1) (eq? (car arg1) '/)  ; (+ (/ a b) (/ c b)) -> (/ (+ a c) b)
				       (pair? arg2) (eq? (car arg2) '/)
				       (pair? (cddr arg1)) (pair? (cddr arg2))
				       (equal? (cddr arg1) (cddr arg2)))
				  `(/ (+ ,(cadr arg1) ,(cadr arg2)) ,@(cddr arg1)))
				 
				 (else `(+ ,@val)))))
			(else 
			 (or (horners-rule val)
			 ;; not many cases here, oddly enough, Horner's rule gets most
			 ;; (+ (/ (f x) 3) (/ (g x) 3) (/ (h x) 3) 15) [ignoring problems involving overflow]
			     `(+ ,@val)))))))))
	    
	    ((*)
	     (case len
	       ((0) 1)
	       ((1) (car args))
	       (else 
		(let ((val (remove-all 1 (splice-if (lambda (x) (eq? x '*)) args))))
		  (if (every? (lambda (x) (or (not (number? x)) (rational? x))) val)
		      (let ((rats (collect-if list rational? val)))
			(if (and (pair? rats)
				 (pair? (cdr rats)))
			    (let ((y (apply * rats)))
			      (set! val (if (= y 1)
					    (collect-if list (lambda (x) (not (number? x))) val)
					    (cons y (collect-if list (lambda (x) (not (number? x))) val))))))))
		  (set! val (remove-inexactions val))
		      
		  (case (length val)
		    ((0) 1)
		    ((1) (car val))                         ; (* x) -> x
		    ((2)
		     (let ((arg1 (car val))
			   (arg2 (cadr val)))

		       (cond ((just-rationals? val)
			      (let ((new-val (apply * val))) ; huge numbers here are less readable
				(if (< (abs new-val) 1000000)
				    new-val
				    `(* ,@val))))

			     ((memv 0 val)                         ; (* x 0) -> 0
			      0) 
			     ((memv -1 val)
			      `(- ,@(remove -1 val)))              ; (* -1 x) -> (- x)

			     ((not (pair? arg2))
			      `(* ,@val))

			     ((pair? arg1)
			      (let ((op1 (car arg1))
				    (op2 (car arg2)))
				(cond ((and (eq? op1 '-)           ; (* (- x) (- y)) -> (* x y)
					    (null? (cddr arg1))
					    (eq? op2 '-)
					    (null? (cddr arg2)))
				       `(* ,(cadr arg1) ,(cadr arg2)))
				      
				      ((and (eq? op1 '/)           ; (* (/ x) (/ y)) -> (/ (* x y)) etc
					    (eq? op2 '/))
				       (let ((op1-arg1 (cadr arg1))
					     (op2-arg1 (cadr arg2)))
					 (if (null? (cddr arg1))
					     (if (null? (cddr arg2))
						 `(/ (* ,op1-arg1 ,op2-arg1))
						 (if (equal? op1-arg1 op2-arg1)
						     `(/ ,(caddr arg2))
						     (simplify-numerics `(/ ,op2-arg1 (* ,op1-arg1 ,(caddr arg2))) env)))
					     (if (null? (cddr arg2))
						 (if (equal? op1-arg1 op2-arg1)
						     `(/ ,(caddr arg1))
						     (simplify-numerics `(/ ,op1-arg1 (* ,(caddr arg1) ,op2-arg1)) env))
						 (simplify-numerics `(/ (* ,op1-arg1 ,op2-arg1) (* ,@(cddr arg1) ,@(cddr arg2))) env)))))
				      
				      ((and (= (length arg1) 3)
					    (equal? (cdr arg1) (cdr arg2))
					    (case op1
					      ((gcd) (eq? op2 'lcm))
					      ((lcm) (eq? op2 'gcd))
					      (else #f)))
				       `(abs (* ,@(cdr arg1))))    ; (* (gcd a b) (lcm a b)) -> (abs (* a b)) but only if 2 args?
				      
				      ((and (eq? op1 'exp)         ; (* (exp a) (exp b)) -> (exp (+ a b))
					    (eq? op2 'exp))
				       `(exp (+ ,(cadr arg1) ,(cadr arg2))))

				      ((and (eq? op1 'sqrt)        ; (* (sqrt x) (sqrt y)) -> (sqrt (* x y))
					    (eq? op2 'sqrt))
				       `(sqrt (* ,(cadr arg1) ,(cadr arg2))))

				      ((not (and (eq? op1 'expt) (eq? op2 'expt)))
				       `(* ,@val))

				      ((equal? (cadr arg1) (cadr arg2)) ; (* (expt x y) (expt x z)) -> (expt x (+ y z))
				       `(expt ,(cadr arg1) (+ ,(caddr arg1) ,(caddr arg2))))

				      ((equal? (caddr arg1) (caddr arg2)) ; (* (expt x y) (expt z y)) -> (expt (* x z) y)
				       `(expt (* ,(cadr arg1) ,(cadr arg2)) ,(caddr arg1)))

				      (else `(* ,@val)))))
			      
			     ((and (number? arg1)                  ; (* 2 (random 3.0)) -> (random 6.0)
				   (eq? (car arg2) 'random)
				   (number? (cadr arg2))
				   (not (rational? (cadr arg2))))
			      `(random ,(* arg1 (cadr arg2))))

			     (else `(* ,@val)))))
		    (else 
		     (cond ((just-rationals? val)
			    (let ((new-val (apply * val))) ; huge numbers here are less readable
			      (if (< (abs new-val) 1000000)
				  new-val
				  `(* ,@val))))

			   ((memv 0 val)                   ; (* x 0 2) -> 0
			    0) 

			   ((memv -1 val)
			    `(- (* ,@(remove -1 val))))    ; (* -1 x y) -> (- (* x y))

			   ((any? (lambda (p)              ; collect * and / vals -> (/ (* ...) ...)
				    (and (pair? p) 
					 (eq? (car p) '/)))
				  val)
			    (let ((mul ())
				  (div ()))
			      (for-each (lambda (p)
					  (if (not (and (pair? p)
							(eq? (car p) '/)))
					      (set! mul (cons p mul))
					      (if (null? (cddr p))
						  (set! div (cons (cadr p) div))
						  (begin
						    (set! mul (cons (cadr p) mul))
						    (set! div (append (cddr p) div))))))
					val)
			      (for-each (lambda (n)
					  (when (member n div)
					    (set! div (remove n div))
					    (set! mul (remove n mul))))
					(copy mul))
			      (let ((expr (if (null? mul)
					      (if (null? div)
						  `(*)       ; for simplify-numerics' benefit
						  `(/ 1 ,@(reverse div)))
					      (if (null? div)
						  `(* ,@(reverse mul))
						  `(/ (* ,@(reverse mul)) ,@(reverse div))))))
				(simplify-numerics expr env))))

			   (else `(* ,@val)))))))))
	    
	    ((-)
	     (set! args (remove-inexactions args))
	     (case len
	       ((0) form)
	       ((1) ; negate
		(if (number? (car args))
		    (- (car args))
		    (if (not (list? (car args)))
			`(- ,@args)
			(case (length (car args))
			  ((2) (if (eq? (caar args) '-)
				   (cadar args)                 ; (- (- x)) -> x
				   `(- ,@args)))
			  ((3) (if (eq? (caar args) '-)
				   `(- ,(caddar args) ,(cadar args)) ; (- (- x y)) -> (- y x)
				   `(- ,@args)))
			  (else `(- ,@args))))))
	       ((2) 
		(let ((arg1 (car args))
		      (arg2 (cadr args)))
		  (cond ((just-rationals? args) (apply - args)) ; (- 3 2) -> 1

			((eqv? arg1 0) `(- ,arg2))              ; (- 0 x) -> (- x)

			((eqv? arg2 0) arg1)                    ; (- x 0) -> x

			((equal? arg1 arg2) 0)                  ; (- x x) -> 0

			((and (pair? arg2)
			      (eq? (car arg2) '-)
			      (pair? (cdr arg2)))
			 (if (null? (cddr arg2)) 
			     `(+ ,arg1 ,(cadr arg2))            ; (- x (- y)) -> (+ x y)
			     (simplify-numerics `(- (+ ,arg1 ,@(cddr arg2)) ,(cadr arg2)) env))) ; (- x (- y z)) -> (- (+ x z) y)

			((and (pair? arg2)                      ; (- x (+ y z)) -> (- x y z)
			      (eq? (car arg2) '+))
			 (simplify-numerics `(- ,arg1 ,@(cdr arg2)) env))

			((and (pair? arg1)                      ; (- (- x y) z) -> (- x y z)
			      (eq? (car arg1) '-))
			 (if (> (length arg1) 2)
			     `(- ,@(cdr arg1) ,arg2)
			     (simplify-numerics `(- (+ ,(cadr arg1) ,arg2)) env)))  ; (- (- x) y) -> (- (+ x y))

			((and (pair? arg2)                      ; (- x (truncate x)) -> (remainder x 1)
			      (eq? (car arg2) 'truncate)
			      (equal? arg1 (cadr arg2)))
			 `(remainder ,arg1 1))

			((and (real? arg2)                      ; (- x -1) -> (+ x 1)
			      (negative? arg2)
			      (not (number? arg1)))
			 `(+ ,arg1 ,(abs arg2)))

			(else `(- ,@args)))))
	       (else 
		(if (just-rationals? args)
		    (apply - args)
		    (let ((val (remove-all 0 (splice-if (lambda (x) (eq? x '+)) (cdr args)))))
		      (if (every? (lambda (x) (or (not (number? x)) (rational? x))) val)
			  (let ((rats (collect-if list rational? val)))
			    (if (and (pair? rats) 
				     (pair? (cdr rats)))
				(let ((y (apply + rats)))
				  (set! val (if (zero? y)
						(collect-if list (lambda (x) (not (number? x))) val)
						(cons y (collect-if list (lambda (x) (not (number? x))) val))))))))
		      (set! val (cons (car args) val))
		      (let ((first-arg (car args))
			    (nargs (cdr val)))
			(if (member first-arg nargs)
			    (begin
			      (set! nargs (remove first-arg nargs)) ; remove once
			      (set! first-arg 0)))
			(cond ((null? nargs) first-arg)       ; (- x 0 0 0)?

			      ((eqv? first-arg 0)
			       (if (null? (cdr nargs))
				   (if (number? (car nargs))
				       (- (car nargs))
				       `(- ,(car nargs)))     ; (- 0 0 0 x)?
				   `(- (+ ,@nargs))))         ; (- 0 z y) -> (- (+ x y))

			      ((not (and (pair? (car args))
					 (eq? (caar args) '-)))
			       `(- ,@(cons first-arg nargs)))

			      ((> (length (car args)) 2)      ; (- (- x y) z w) -> (- x y z w)
			       (simplify-numerics `(- ,@(cdar args) ,@(cdr args)) env))

			      (else (simplify-numerics `(- (+ ,(cadar args) ,@(cdr args))) env)))))))))

	    ((/)
	     (set! args (remove-inexactions args))
	     (case len
	       ((0) form)
	       ((1) ; invert
		(if (number? (car args))
		    (if (zero? (car args))
			`(/ ,(car args))
			(/ (car args)))
		    (if (not (pair? (car args)))
			`(/ ,@args)
			(case (caar args)
			  ((/) 
			   (case (length (car args))
			     ((2)                         ; (/ (/ x)) -> x
			      (cadar args))
			     ((3)                         ; (/ (/ z x)) -> (/ x z)
			      `(/ ,@(reverse (cdar args))))
			     (else
			      (if (eqv? (cadar args) 1)
				  `(* ,@(cddar args))     ; (/ (/ 1 x y)) -> (* x y)
				  `(/ (* ,@(cddar args)) ,(cadar args)))))) ; (/ (/ z x y)) -> (/ (* x y) z)
			  ((expt)                         ; (/ (expt x y)) -> (expt x (- y))
			   `(expt ,(cadar args) (- ,(caddar args))))
			  ((exp)                          ; (/ (exp x)) -> (exp (- x))
			   `(exp (- ,(cadar args))))
			  (else `(/ ,@args))))))
	       ((2)
		(if (and (just-rationals? args)
			 (not (zero? (cadr args))))
		    (apply / args)                         ; including (/ 0 12) -> 0
		    (let ((arg1 (car args))
			  (arg2 (cadr args)))
		      (let ((op1 (and (pair? arg1) (car arg1)))
			    (op2 (and (pair? arg2) (car arg2))))
			(let ((op1-arg1 (and op1 (pair? (cdr arg1)) (cadr arg1)))
			      (op2-arg1 (and op2 (pair? (cdr arg2)) (cadr arg2))))
			  (cond ((eqv? arg1 1)                 ; (/ 1 x) -> (/ x)
				 (simplify-numerics `(/ ,arg2) env))
				
				((eqv? arg2 1)                 ; (/ x 1) -> x
				 arg1)
				
				((and (pair? arg1)             ; (/ (/ a b) c) -> (/ a b c)
				      (eq? op1 '/)
				      (pair? (cddr arg1))
				      (not (and (pair? arg2)
						(eq? op2 '/))))
				 `(/ ,op1-arg1 ,@(cddr arg1) ,arg2))
				
				((and (pair? arg1)             ; (/ (/ a) (/ b)) -> (/ b a)??
				      (eq? op1 '/)
				      (pair? arg2)
				      (eq? '/ op2))
				 (let ((a1 (if (null? (cddr arg1)) (list 1 op1-arg1) (cdr arg1)))
				       (a2 (if (null? (cddr arg2)) (list 1 op2-arg1) (cdr arg2))))
				   (simplify-numerics `(/ (* ,(car a1) ,@(cdr a2)) (* ,@(cdr a1) ,(car a2))) env)))
				
				((and (pair? arg2)
				      (eq? op2 '*)
				      (not (side-effect? arg1 env))
				      (member arg1 (cdr arg2)))
				 (let ((n (remove arg1 (cdr arg2))))
				   (if (and (pair? n) (null? (cdr n)))
				       `(/ ,@n)               ; (/ x (* y x)) -> (/ y)
				       `(/ 1 ,@n))))          ; (/ x (* y x z)) -> (/ 1 y z)
				
				((and (pair? arg2)            ; (/ c (/ a b)) -> (/ (* c b) a)
				      (eq? op2 '/))
				 (cond ((null? (cddr arg2))
					`(* ,arg1 ,op2-arg1))  ; ignoring divide by zero here (/ x (/ y)) -> (* x y)				 
				       ((eqv? op2-arg1 1)
					`(* ,arg1 ,@(cddr arg2)))  ; (/ x (/ 1 y z)) -> (* x y z) -- these never actually happen
				       ((not (pair? (cddr arg2)))
					`(/ ,@args))               ; no idea...
				       ((and (rational? arg1)
					     (rational? op2-arg1)
					     (null? (cdddr arg2)))
					(let ((val (/ arg1 op2-arg1)))
					  (if (= val 1)
					      (caddr arg2)
					      (if (= val -1)
						  `(- ,(caddr arg2))
						  `(* ,val ,(caddr arg2))))))
				       (else `(/ (* ,arg1 ,@(cddr arg2)) ,op2-arg1))))
#|				
				;; can't decide about this -- result usually looks cruddy
				((and (pair? arg2)             ; (/ x (* y z)) -> (/ x y z)
				      (eq? op2 '*))
				 `(/ ,arg1 ,@(cdr arg2)))
|#				
				((and (pair? arg1)             ; (/ (log x) (log y)) -> (log x y) -- (log number) for (log y) never happens
				      (pair? arg2)
				      (= (length arg1) (length arg2) 2)
				      (case op1
					((log) (eq? op2 'log))
					((sin)
					 (and (eq? op2 'cos)
					      (equal? op1-arg1 op2-arg1)))
					(else #f)))
				 (if (eq? op1 'log)
				     `(log ,op1-arg1 ,op2-arg1)
				     `(tan ,op1-arg1)))
				
				((and (pair? arg1)             ; (/ (- x) (- y)) -> (/ x y)
				      (pair? arg2)
				      (eq? op1 '-)
				      (eq? op2 '-)
				      (= (length arg1) (length arg2) 2))
				 `(/ ,op1-arg1 ,op2-arg1))
				
				((and (pair? arg1)             ; (/ (* x y) (* z y)) -> (/ x z)
				      (pair? arg2)
				      (eq? op1 '*)
				      (case op2
					((*)
					 (and (= (length arg1) (length arg2) 3)
					      (equal? (caddr arg1) (caddr arg2))))
					((log)
					 (cond ((assq 'log (cdr arg1)) 
						=> (lambda (p)
						     (= (length p) 2)))
					       (else #f)))
					(else #f))           ; (/ (* 12 (log x)) (log 2)) -> (* 12 (log x 2))
				      (if (eq? op2 '*)
					  `(/ ,op1-arg1 ,op2-arg1)
					  (let ((used-log op2-arg1))
					    `(* ,@(map (lambda (p)
							 (if (and used-log
								  (pair? p)
								  (eq? (car p) 'log))
							     (let ((val `(log ,(cadr p) ,used-log)))
							       (set! used-log #f)
							       val)
							     p))
						       (cdr arg1)))))))

				((and (pair? arg1)            ; (/ (sqrt x) x) -> (/ (sqrt x))
				      (eq? (car arg1) 'sqrt)
				      (equal? (cadr arg1) arg2))
				 `(/ ,arg1))
				
				((and (pair? arg2)            ; (/ x (sqrt x)) -> (sqrt x)
				      (eq? (car arg2) 'sqrt)
				      (equal? (cadr arg2) arg1))
				 arg2)
				
				(else `(/ ,@args))))))))

	       (else 
		(if (and (just-rationals? args)
			 (not (memv 0 (cdr args)))
			 (not (memv 0.0 (cdr args))))
		    (apply / args)
		    (let ((nargs                            ; (/ x a (* b 1 c) d) -> (/ x a b c d)
			   (remove-all 1 (splice-if (lambda (x) (eq? x '*)) (cdr args)))))
		      (if (null? nargs) ; (/ x 1 1) -> x
			  (car args)
			  (if (and (member (car args) (cdr args))
				   (not (side-effect? (car args) env)))
			      (let ((n (remove (car args) (cdr args))))
				(if (null? (cdr n))
				    `(/ ,@n)                ; (/ x y x) -> (/ y)
				    `(/ 1 ,@n)))            ; (/ x y x z) -> (/ 1 y z)
			      `(/ ,@(cons (car args) nargs)))))))))
	    
	    ((sin cos tan asin acos sinh cosh tanh asinh acosh atanh exp)
	     ;; perhaps someday, for amusement:
	     ;;    (sin (acos x)) == (cos (asin x)) == (sqrt (- 1 (expt x 2)))
	     ;;    (asin (cos x)) == (acos (sin x)) == (- (* 1/2 pi) x)

	     (cond ((not (= len 1))
		    `(,(car form) ,@args))
		   ((and (pair? (car args))                 ; (sin (asin x)) -> x
			 (= (length (car args)) 2)
			 (eq? (caar args) (inverse-op (car form))))
		    (cadar args))
		   ((eqv? (car args) 0)                     ; (sin 0) -> 0
		    (case (car form)
		      ((sin asin sinh asinh tan tanh atanh) 0)
		      ((exp cos cosh) 1)
		      (else `(,(car form) ,@args))))
		   ((and (eq? (car form) 'cos)              ; (cos (- x)) -> (cos x)
			 (pair? (car args))
			 (eq? (caar args) '-)
			 (null? (cddar args)))
		    `(cos ,(cadar args)))
		   ((or (eq? (car args) 'pi)                ; (sin pi) -> 0.0
			(and (pair? (car args))
			     (eq? (caar args) '-)
			     (eq? (cadar args) 'pi)
			     (null? (cddar args))))
		    (case (car form)
		      ((sin tan) 0.0)
		      ((cos) -1.0)
		      (else `(,(car form) ,@args))))
		   ((eqv? (car args) 0.0)                   ; (sin 0.0) -> 0.0
		    ((symbol->value (car form)) 0.0))
		   ((and (eq? (car form) 'acos)             ; (acos -1) -> pi
			 (eqv? (car args) -1))
		    'pi)
		   ((and (eq? (car form) 'exp)              ; (exp (* a (log b))) -> (expt b a)
			 (pair? (car args))
			 (eq? (caar args) '*))
		    (let ((targ (cdar args)))
		      (cond ((not (= (length targ) 2))
			     `(,(car form) ,@args))
			    ((and (pair? (car targ))
				  (eq? (caar targ) 'log)
				  (pair? (cdar targ))
				  (null? (cddar targ)))
			     `(expt ,(cadar targ) ,(cadr targ)))
			    ((and (pair? (cadr targ))
				  (eq? (caadr targ) 'log) 
				  (pair? (cdadr targ))
				  (null? (cddadr targ)))
			     `(expt ,(cadadr targ) ,(car targ)))
			    (else `(,(car form) ,@args)))))
		   (else `(,(car form) ,@args))))
	    
	    ((log)
	     (cond ((not (pair? args)) form)
		   ((eqv? (car args) 1) 0)      ; (log 1 ...) -> 0
		   ((and (= len 1)              ; (log (exp x)) -> x
			 (pair? (car args))
			 (= (length (car args)) 2)
			 (eq? (caar args) 'exp))
		    (cadar args))
		   ((and (pair? (car args))     ; (log (sqrt x)) -> (* 1/2 (log x))
			 (eq? (caar args) 'sqrt))
		    `(* 1/2 (log ,(cadar args) ,@(cdr args))))
		   ((and (pair? (car args))     ; (log (expt x y)) -> (* y (log x))
			 (eq? (caar args) 'expt))
		    `(* ,(caddar args) (log ,(cadar args) ,@(cdr args))))
		   ((not (and (= len 2)         ; (log x x) -> 1.0
			      (equal? (car args) (cadr args))))
		    `(log ,@args))
		   ((integer? (car args)) 1)
		   (else 1.0)))
	    
	    ((sqrt)
	     (cond ((not (pair? args))
		    form)
		   ((and (rational? (car args))
			 (rational? (sqrt (car args)))
			 (= (car args) (sqrt (* (car args) (car args)))))
		    (sqrt (car args))) ; don't collapse (sqrt (* a a)), a=-1 for example, or -1-i -> 1+i whereas 1-i -> 1-i etc
		   ((and (pair? (car args))
			 (eq? (caar args) 'exp))
		    `(exp (/ ,(cadar args) 2))) ; (sqrt (exp x)) -> (exp (/ x 2))
		   (else `(sqrt ,@args))))
	    
	    ((floor round ceiling truncate)
	     (cond ((not (= len 1))
		    form)

		   ((number? (car args))
		    (catch #t 
		      (lambda () (apply (symbol->value (car form)) args)) 
		      (lambda any `(,(car form) ,@args))))

		   ((not (pair? (car args)))
		    `(,(car form) ,@args))

		   ((or (integer-result? (caar args))
			(and (eq? (caar args) 'random)
			     (integer? (cadar args))))
		    (car args))
			  
		   ((memq (caar args) '(inexact->exact exact))
		    `(,(car form) ,(cadar args)))
			  
		   ((memq (caar args) '(* + / -)) ; maybe extend this list
		    `(,(car form) (,(caar args) ,@(map (lambda (p)
							 (if (and (pair? p)
								  (memq (car p) '(inexact->exact exact)))
							     (cadr p)
							     p))
						       (cdar args)))))
		   ((and (eq? (caar args) 'random)
			 (eq? (car form) 'floor)
			 (float? (cadar args))
			 (= (floor (cadar args)) (cadar args)))
		    `(random ,(floor (cadar args))))
			  
		   (else `(,(car form) ,@args))))
	    
	    ((abs magnitude)
	     (cond ((not (= len 1))
		    form)

		   ((and (pair? (car args))        ; (abs (abs x)) -> (abs x)
			 (hash-table-ref non-negative-ops (caar args)))
		    (car args))

		   ((rational? (car args))
		    (abs (car args)))

		   ((not (pair? (car args)))
		    `(,(car form) ,@args))

		   ((and (memq (caar args) '(modulo random))
			 (= (length (car args)) 3) ; (abs (modulo x 2)) -> (modulo x 2)
			 (real? (caddar args))
			 (positive? (caddar args)))
		    (car args))

		   ((and (eq? (caar args) '-)      ; (abs (- x)) -> (abs x)
			 (pair? (cdar args))
			 (null? (cddar args)))
		    `(,(car form) ,(cadar args)))

		   (else `(,(car form) ,@args))))
	
	    ((imag-part)
	     (if (not (= len 1))
		 form
		 (if (or (real? (car args))
			 (and (pair? (car args))
			      (real-result? (caar args))))
		     0.0
		     `(imag-part ,@args))))
	    
	    ((real-part)
	     (if (not (= len 1))
		 form
		 (if (or (real? (car args))
			 (and (pair? (car args))
			      (real-result? (caar args))))
		     (car args)
		     `(real-part ,@args))))
	    
	    ((denominator)
	     (if (not (= len 1))
		 form
		 (if (or (integer? (car args))
			 (and (pair? (car args))
			      (integer-result? (caar args))))
		     1
		     `(denominator ,(car args)))))
	    
	    ((numerator)
	     (cond ((not (= len 1))
		    form)
		   ((or (integer? (car args))
			(and (pair? (car args))
			     (integer-result? (caar args))))
		    (car args))
		   ((rational? (car args))
		    (numerator (car args)))
		   (else `(numerator ,(car args)))))
	    
	    ((random)
	     (cond ((not (and (= len 1)
			      (number? (car args))))
		    `(random ,@args))  
		   ((eqv? (car args) 0)
		    0)
		   ((morally-equal? (car args) 0.0)
		    0.0)
		   (else `(random ,@args))))
	    
	    ((complex make-rectangular)
	     (if (and (= len 2)
		      (morally-equal? (cadr args) 0.0)) ; morally so that 0 matches
		 (car args)
		 `(complex ,@args)))
	    
	    ((make-polar)
	     (if (and (= len 2)
		      (morally-equal? (cadr args) 0.0))
		 (car args)
		 `(make-polar ,@args)))

	    ((rationalize lognot ash modulo remainder quotient)
	     (cond ((just-rationals? args)
		    (catch #t ; catch needed here for things like (ash 2 64)
		      (lambda ()
			(apply (symbol->value (car form)) args))
		      (lambda ignore
			`(,(car form) ,@args)))) ; use this form to pick up possible arg changes
		   
		   ((and (eq? (car form) 'ash)          ; (ash x 0) -> x
			 (= len 2) ; length of args
			 (eqv? (cadr args) 0))
		    (car args))
		   
		   ((case (car form)
		      ((quotient)                       ; (quotient (remainder x y) y) -> 0
		       (and (= len 2)
			    (pair? (car args))
			    (eq? (caar args) 'remainder)
			    (= (length (car args)) 3)
			    (eqv? (caddar args) (cadr args))))
		      ((ash modulo)                     ; (modulo 0 x) -> 0
		       (and (= len 2) (eqv? (car args) 0)))
		      (else #f))
		    0)
		   
		   ((and (eq? (car form) 'modulo)       ; (modulo (abs x) y) -> (modulo x y)
			 (= len 2)
			 (pair? (car args))
			 (eq? (caar args) 'abs))
		    `(modulo ,(cadar args) ,(cadr args)))
		   
		   (else `(,(car form) ,@args))))
	    
	    ((expt) 
	     (cond ((not (= len 2))
		    form)
		   ((and (eqv? (car args) 0)            ; (expt 0 x) -> 0
			 (not (eqv? (cadr args) 0)))
		    (if (and (integer? (cadr args))
			     (negative? (cadr args)))
			(lint-format "attempt to divide by 0: ~A" 'expt (truncated-list->string form)))
		    0)
		   ((or (and (eqv? (cadr args) 0)       ; (expt x 0) -> 1
			     (not (eqv? (car args) 0)))
			(eqv? (car args) 1))            ; (expt 1 x) -> 1    
		    1)
		   ((eqv? (cadr args) 1)                ; (expt x 1) -> x
		    (car args))
		   ((eqv? (cadr args) -1)               ; (expt x -1) -> (/ x)
		    `(/ ,(car args)))
		   ((just-rationals? args)              ; (expt 2 3) -> 8
		    (catch #t
		      (lambda ()
			(let ((val (apply expt args)))
			  (if (and (integer? val)
				   (< (abs val) 1000000))
			      val
			      `(expt ,@args))))
		      (lambda args
			`(expt ,@args))))               ; (expt (expt x y) z) -> (expt x (* y z))
		   ((and (pair? (car args))
			 (eq? (caar args) 'expt))
		    `(expt ,(cadar args) (* ,(caddar args) ,(cadr args))))
		   (else `(expt ,@args))))

	    
	    ((angle)
	     (cond ((not (pair? args)) form)
		   ((eqv? (car args) -1) 'pi)
		   ((or (morally-equal? (car args) 0.0)
			(eq? (car args) 'pi))
		    0.0)
		   (else `(angle ,@args))))
	    
	    ((atan)
	     (cond ((and (= len 1)                    ; (atan (x y)) -> (atan x y)
			 (pair? (car args))
			 (= (length (car args)) 3)
			 (eq? (caar args) '/))
		    `(atan ,@(cdar args)))
		   ((and (= len 2)                    ; (atan 0 -1) -> pi
			 (eqv? (car args) 0)
			 (eqv? (cadr args) -1))
		    'pi)
		   (else `(atan ,@args))))
	    
	    ((inexact->exact exact)
	     (cond ((not (= len 1)) 
		    form)
		   ((or (rational? (car args))
			(and (pair? (car args))
			     (or (rational-result? (caar args))
				 (integer-result? (caar args))
				 (and (eq? (caar args) 'random)
				      (rational? (cadar args))))))
		    (car args))
		   ((number? (car args))
		    (catch #t (lambda () (inexact->exact (car args))) (lambda any `(,(car form) ,@args))))
		   (else `(,(car form) ,@args))))
	    
	    ((exact->inexact inexact)
	     (cond ((not (= len 1))
		    form)

		   ((memv (car args) '(0 0.0))
		    0.0)

		   ((not (and (pair? (car args))
			      (not (eq? (caar args) 'random))
			      (hash-table-ref numeric-ops (caar args))
			      (any? number? (cdar args))))
		    `(,(car form) ,@args))

		    ((any? (lambda (x)
			     (and (number? x)
				  (inexact? x)))
			   (cdar args))
		     (car args))

		    (else
		     (let ((new-form (copy (car args))))
		       (do ((p (cdr new-form) (cdr p)))
			   ((or (null? p)
				(number? (car p)))
			    (if (pair? p)
				(set-car! p (* 1.0 (car p))))
			    new-form))))))
		   ;; not (inexact (random 3)) -> (random 3.0) because results are different
	    
	    ((logior)
	     (set! args (lint-remove-duplicates (remove-all 0 (splice-if (lambda (x) (eq? x 'logior)) args)) env))
	     (if (every? (lambda (x) (or (not (number? x)) (integer? x))) args)
		 (let ((rats (collect-if list integer? args)))
		   (if (and (pair? rats) 
			    (pair? (cdr rats)))
		       (let ((y (apply logior rats)))
			 (set! args (if (zero? y)
					(collect-if list (lambda (x) (not (number? x))) args)
					(cons y (collect-if list (lambda (x) (not (number? x))) args))))))))
	     (cond ((null? args) 0)                ; (logior) -> 0
		   ((null? (cdr args)) (car args)) ; (logior x) -> x
		   ((memv -1 args) -1)             ; (logior ... -1 ...) -> -1
		   ((just-integers? args) (apply logior args))
		   (else `(logior ,@args))))
	    
	    ((logand)
	     (set! args (lint-remove-duplicates (remove-all -1 (splice-if (lambda (x) (eq? x 'logand)) args)) env))
	     (if (every? (lambda (x) (or (not (number? x)) (integer? x))) args)
		 (let ((rats (collect-if list integer? args)))
		   (if (and (pair? rats) 
			    (pair? (cdr rats)))
		       (let ((y (apply logand rats)))
			 (set! args (if (= y -1)
					(collect-if list (lambda (x) (not (number? x))) args)
					(cons y (collect-if list (lambda (x) (not (number? x))) args))))))))
	     (cond ((null? args) -1)
		   ((null? (cdr args)) (car args)) ; (logand x) -> x
		   ((memv 0 args) 0)
		   ((just-integers? args) (apply logand args))
		   (else `(logand ,@args))))
	    
	    ;; (logand 1 (logior 2 x)) -> (logand 1 x)? 
	    ;; (logand 1 (logior 1 x)) -> 1
	    ;; (logand 3 (logior 1 x))?
	    ;; similarly for (logior...(logand...))
	    
	    ((logxor)
	     (set! args (splice-if (lambda (x) (eq? x 'logxor)) args)) ; is this correct??
	     (cond ((null? args) 0)                                    ; (logxor) -> 0
		   ((null? (cdr args)) (car args))                     ; (logxor x) -> x??
		   ((just-integers? args) (apply logxor args))         ; (logxor 1 2) -> 3
		   ((and (= len 2) (equal? (car args) (cadr args))) 0) ; (logxor x x) -> 0
		   (else `(logxor ,@args))))                           ; (logxor x (logxor y z)) -> (logxor x y z)
	    
	    ((gcd)
	     (set! args (lint-remove-duplicates (splice-if (lambda (x) (eq? x 'gcd)) args) env))
	     (cond ((null? args) 0) 
		   ((memv 1 args) 1)
		   ((just-integers? args)
		    (catch #t  ; maybe (gcd -9223372036854775808 -9223372036854775808)
		      (lambda ()
			(apply gcd args))
		      (lambda ignore
			`(gcd ,@args))))
		   ((null? (cdr args))   `(abs ,(car args)))
		   ((eqv? (car args) 0)  `(abs ,(cadr args)))
		   ((eqv? (cadr args) 0) `(abs ,(car args)))
		   (else `(gcd ,@args))))
	    
	    ((lcm)
	     (set! args (lint-remove-duplicates (splice-if (lambda (x) (eq? x 'lcm)) args) env))
	     (cond ((null? args) 1)         ; (lcm) -> 1
		   ((memv 0 args) 0)        ; (lcm ... 0 ...) -> 0
		   ((just-integers? args)   ; (lcm 3 4) -> 12
		    (catch #t
		      (lambda ()
			(apply lcm args))
		      (lambda ignore
			`(lcm ,@args))))
		   ((null? (cdr args))      ; (lcm x) -> (abs x)
		    `(abs ,(car args)))
		   (else `(lcm ,@args))))
	    
	    ((max min)
	     (if (not (pair? args))
		 form
		 (begin
		   (set! args (lint-remove-duplicates (splice-if (lambda (x) (eq? x (car form))) args) env))
		   (if (any? (lambda (p) ; if non-negative-op, remove any non-positive numbers
			       (and (pair? p)
				    (hash-table-ref non-negative-ops (car p))))
			     args)
		       (set! args (remove-if (lambda (x)
					       (and (real? x)
						    (not (positive? x))))
					     args)))
		   (if (= len 1)
		       (car args)
		       (if (just-reals? args)
			   (apply (symbol->value (car form)) args)
			   (let ((nums (collect-if list number? args))
				 (other (if (eq? (car form) 'min) 'max 'min)))
			     (if (and (pair? nums)
				      (just-reals? nums)) ; non-real case checked elsewhere (later)
				 (let ((relop (if (eq? (car form) 'min) >= <=)))
				   (if (pair? (cdr nums))
				       (set! nums (list (apply (symbol->value (car form)) nums))))
				   (let ((new-args (append nums (collect-if list (lambda (x) (not (number? x))) args))))
				     (let ((c1 (car nums)))
				       (set! new-args (collect-if list (lambda (x)
									 (or (not (pair? x))
									     (<= (length x) 2)
									     (not (eq? (car x) other))
									     (let ((c2 (find-if number? (cdr x))))
									       (or (not c2)
										   (relop c1 c2)))))
								  new-args)))
				     (if (< (length new-args) (length args))
					 (set! args new-args)))))

			     ;; if (max c1 (min c2 . args1) . args2) where (> c1 c2) -> (max c1 . args2), if = -> c1
			     ;; if (min c1 (max c2 . args1) . args2) where (< c1 c2) -> (min c1 . args2), if = -> c1
			     ;;   and if (max 4 x (min x 4)) -- is it (max x 4)?
			     ;; (max a b) is (- (min (- a) (- b))), but that doesn't help here -- the "-" gets in our way
			     ;;   (min (- a) (- b)) -> (- (max a b))?
			     ;; (+ a (max|min b c)) = (max|min (+ a b) (+ a c)))

			     (if (null? (cdr args)) ; (max (min x 3) (min x 3)) -> (max (min x 3)) -> (min x 3)
				 (car args)
				 (if (and (null? (cddr args))   ; (max|min x (min|max x ...) -> x
					  (or (and (pair? (car args))
						   (eq? (caar args) other)
						   (member (cadr args) (car args))
						   (not (side-effect? (cadr args) env)))
					      (and (pair? (cadr args))
						   (eq? (caadr args) other)
						   (member (car args) (cadr args))
						   (not (side-effect? (car args) env)))))
				     ((if (pair? (car args)) cadr car) args)
				     `(,(car form) ,@args)))))))))
	    (else 
	     `(,(car form) ,@args))))))
    
    
    (define (binding-ok? caller head binding env second-pass)
      ;; check let-style variable binding for various syntactic problems
      (cond (second-pass
	     (and (pair? binding)
		  (symbol? (car binding))
		  (not (constant? (car binding)))
		  (pair? (cdr binding))
		  (or (null? (cddr binding))
		      (and (eq? head 'do)
			   (pair? (cddr binding)) ; (do ((i 0 . 1))...)
			   (null? (cdddr binding))))))
	  
	    ((not (pair? binding)) 	   (lint-format "~A binding is not a list? ~S" caller head binding) #f)    ; (let (a) a)
	    ((not (symbol? (car binding))) (lint-format "~A variable is not a symbol? ~S" caller head binding) #f) ; (let ((1 2)) #f)
	    ((keyword? (car binding))	   (lint-format "~A variable is a keyword? ~S" caller head binding) #f)    ; (let ((:a 1)) :a)
	    ((constant? (car binding))	   (lint-format "can't bind a constant: ~S" caller binding) #f)            ; (let ((pi 2)) #f)
	    ((not (pair? (cdr binding)))
	     (lint-format (if (null? (cdr binding))
			      "~A variable value is missing? ~S"     ; (let ((a)) #f)
			      "~A binding is an improper list? ~S")  ; (let ((a . 1)) #f)
			  caller head binding)
	     #f)
	    ((and (pair? (cddr binding))               ; (let loop ((pi 1.0) (+ pi 1))...)
		  (or (not (eq? head 'do))
		      (pair? (cdddr binding))))
	     (lint-format "~A binding is messed up: ~A" caller head binding)
	     #f)
	    (else 
	     (if (and *report-shadowed-variables*      ; (let ((x 1)) (+ (let ((x 2)) (+ x 1)) x))
		      (var-member (car binding) env))
		 (lint-format "~A variable ~A in ~S shadows an earlier declaration" caller head (car binding) binding))
	     #t)))

    (define (check-char-cmp caller op form)
      (if (and (any? (lambda (x) 
		       (and (pair? x) 
			    (eq? (car x) 'char->integer)))
		     (cdr form))
	       (every? (lambda (x) 
			 (or (and (integer? x)
				  (<= 0 x 255))
			     (and (pair? x) 
				  (eq? (car x) 'char->integer))))
		       (cdr form)))
	  (lint-format "perhaps ~A" caller             ; (< (char->integer x) 95) -> (char<? x #\_)
		       (lists->string form
				      `(,(case op ((=) 'char=?) ((>) 'char>?) ((<) 'char<?) ((>=) 'char>=?) (else 'char<=?))
					,@(map (lambda (arg)
						 ((if (integer? arg) integer->char cadr) arg))
					       (cdr form)))))))
    
    (define (write-port expr) ; ()=not specified (*stdout*), #f=something is wrong (not enough args)
      (and (pair? expr)
	   (if (eq? (car expr) 'newline)
	       (if (pair? (cdr expr))
		   (cadr expr)
		   ())
	       (and (pair? (cdr expr))
		    (if (pair? (cddr expr))
			(caddr expr)
			())))))
    
    (define (display->format d)
      (case (car d)
	((newline) (copy "~%"))
	
	((display) 
	 (let* ((arg (cadr d))
		(arg-arg (and (pair? arg)
			      (pair? (cdr arg))
			      (cadr arg))))
	   (cond ((string? arg)
		  arg)

		 ((char? arg)
		  (string arg))

		 ((and (pair? arg)
		       (eq? (car arg) 'number->string))
		  (if (= (length arg) 3)
		      (case (caddr arg)
			((2) (values "~B" arg-arg))
			((8) (values "~O" arg-arg))
			((10) (values "~D" arg-arg))
			((16) (values "~X" arg-arg))
			(else (values "~A" arg)))
		      (values "~A" arg-arg)))

		 ((not (and (pair? arg)
			    (eq? (car arg) 'string-append)))
		  (values "~A" arg))

		 ((null? (cddr arg))
		  (if (string? arg-arg)
		      arg-arg
		      (values "~A" arg-arg)))

		 ((not (null? (cdddr arg)))
		  (values "~A" arg))

		 ((string? arg-arg)
		  (values (string-append arg-arg "~A") (caddr arg)))

		 ((string? (caddr arg))
		  (values (string-append "~A" (caddr arg)) arg-arg))

		 (else (values "~A" arg)))))
	
	((write)
	 ;; very few special cases actually happen here, unlike display above
	 (if (string? (cadr d))
	     (string-append "\"" (cadr d) "\"")
	     (if (char? (cadr d))
		 (string (cadr d))
		 (values "~S" (cadr d)))))
	
	((write-char)
	 (if (char? (cadr d))
	     (string (cadr d))
	     (values "~C" (cadr d))))
	
	((write-string)  ; same as display but with possible start|end indices
	 (let ((indices (and (pair? (cddr d)) ; port
			     (pair? (cdddr d))
			     (cdddr d))))
	   (if (string? (cadr d))
	       (if (not indices)
		   (cadr d)
		   (if (and (integer? (car indices))
			    (or (null? (cdr indices))
				(and (pair? indices)
				     (integer? (cadr indices)))))
		       (apply substring (cadr d) indices)
		       (values "~A" `(substring ,(cadr d) ,@indices))))
	       (values "~A" (if indices `(substring ,(cadr d) ,@indices) (cadr d))))))))
    
    (define (identity? x) ; (lambda (x) x), or (define (x) x) -> procedure-source
      (and (pair? x)
	   (eq? (car x) 'lambda)
	   (pair? (cdr x))
	   (pair? (cadr x))
	   (null? (cdadr x))
	   (pair? (cddr x))
	   (null? (cdddr x))
	   (eq? (caddr x) (caadr x))))
    
    (define (cdr-count c)
      (case c ((cdr) 1) ((cddr) 2) ((cdddr) 3) (else 4)))
    
    (define (simple-lambda? x)
      (and (pair? x)
	   (eq? (car x) 'lambda)
	   (pair? (cdr x))
	   (pair? (cadr x))
	   (null? (cdadr x))
	   (pair? (cddr x))
	   (null? (cdddr x))
	   (= (tree-count1 (caadr x) (caddr x) 0) 1)))
    
    (define (less-simple-lambda? x)
      (and (pair? x)
	   (eq? (car x) 'lambda)
	   (pair? (cdr x))
	   (pair? (cadr x))
	   (null? (cdadr x))
	   (pair? (cddr x))
	   (= (tree-count1 (caadr x) (cddr x) 0) 1)))
    
    (define (tree-subst new old tree)
      (cond ((equal? old tree)
	     new)

	    ((not (pair? tree))
	     tree)

	    ((eq? (car tree) 'quote)
	     (copy-tree tree))
	    
	    (else (cons (tree-subst new old (car tree))
			(tree-subst new old (cdr tree))))))

    
    (define* (find-unique-name f1 f2 (i 1))
      (let ((sym (string->symbol (format #f "_~D_" i))))
	(if (not (or (eq? sym f1)
		     (eq? sym f2)
		     (tree-member sym f1)
		     (tree-member sym f2)))
	    sym
	    (find-unique-name f1 f2 (+ i 1)))))
    
    (define (unrelop caller head form)         ; assume len=3 
      (let ((arg1 (cadr form))
	    (arg2 (caddr form)))
	(if (and (pair? arg1)
		 (= (length arg1) 3))
	    (if (eq? (car arg1) '-)
		(if (memv arg2 '(0 0.0))               ; (< (- x y) 0) -> (< x y), need both 0 and 0.0 because (eqv? 0 0.0) is #f
		    (lint-format "perhaps ~A" caller 
				 (lists->string form 
						`(,head ,(cadr arg1) ,(caddr arg1))))
		    (if (and (integer? arg2)           ; (> (- x 50868) 256) -> (> x 51124)
			     (integer? (caddr arg1)))
			(lint-format "perhaps ~A" caller 
				     (lists->string form 
						    `(,head ,(cadr arg1) ,(+ (caddr arg1) arg2))))))
		;; (> (- x) (- y)) (> (- x 1) (- y 1)) and so on -- do these ever happen? (no, not even if we allow +-*/)
			
		(if (and (eq? (car arg1) '+)           ; (< (+ x 1) 3) -> (< x 2)
			 (integer? arg2)  
			 (integer? (caddr arg1)))
		    (lint-format "perhaps ~A" caller 
				 (lists->string form 
						`(,head ,(cadr arg1) ,(- arg2 (caddr arg1)))))))
	    (if (and (pair? arg2)
		     (= (length arg2) 3))
		(if (eq? (car arg2) '-)
		    (if (memv arg1 '(0 0.0))           ; (< 0 (- x y)) -> (> x y)
			(lint-format "perhaps ~A" caller 
				     (lists->string form 
						    `(,(hash-table-ref reversibles head) 
						      ,(cadr arg2) ,(caddr arg2))))
			(if (and (integer? arg1)
				 (integer? (caddr arg2)))
			    (lint-format "perhaps ~A" caller 
					 (lists->string form 
							`(,(hash-table-ref reversibles head) 
							  ,(cadr arg2) ,(+ arg1 (caddr arg2)))))))
		    (if (and (eq? (car arg2) '+)       ; (< 256 (+ fltdur 50868)) -> (> fltdur -50612)
			     (integer? arg1)
			     (integer? (caddr arg2)))
			(lint-format "perhaps ~A" caller 
				     (lists->string form 
						    `(,(hash-table-ref reversibles head) 
						      ,(cadr arg2) ,(- arg1 (caddr arg2)))))))))))
			
    
    (define (check-start-and-end caller head form ff env)
      (if (or (and (integer? (car form))
		   (integer? (cadr form))
		   (apply >= form))
	      (and (equal? (car form) (cadr form))
		   (not (side-effect? (car form) env))))
	  (lint-format "these ~A indices make no sense: ~A" caller head ff)))  ; (copy x y 1 0)

    (define (other-case c)
      ((if (char-upper-case? c) char-downcase char-upcase) c))
    
    (define (check-boolean-affinity caller form env)
      ;; does built-in boolean func's arg make sense
      (if (and (= (length form) 2)
	       (not (symbol? (cadr form)))
	       (not (= line-number last-simplify-boolean-line-number)))
	  (let ((expr (simplify-boolean form () () env)))
	    (if (not (equal? expr form))
		(lint-format "perhaps ~A" caller (lists->string form expr)))  ; (char? '#\a) -> #t
	    (if (and (pair? (cadr form))
		     (symbol? (caadr form)))
		(let ((rt (if (eq? (caadr form) 'quote)
			      (->simple-type (cadadr form))
			      (return-type (caadr form) env)))
		      (head (car form)))
		  (if (subsumes? head rt)
		      (lint-format "~A is always #t" caller (truncated-list->string form)) ; (char? '#\a) is always #t
		      (if (not (or (memq rt '(#t #f values))
				   (any-compatible? head rt)))
			  (lint-format "~A is always #f" caller (truncated-list->string form))))))))) ; (number? (make-list 1)) is always #f


    (define combinable-cxrs (let ((h (make-hash-table)))
			      (for-each (lambda (c)
					  (hash-table-set! h c (let ((name (symbol->string c)))
								 (substring name 1 (- (length name) 1)))))
					'(car cdr caar cadr cddr cdar caaar caadr caddr cdddr cdaar cddar cadar cdadr cadddr cddddr))
			      h))
    ;; not combinable: caaaar caaadr caadar caaddr cadaar cadadr caddar cdaaar cdaadr cdadar cdaddr cddaar cddadr cdddar

    (define (combine-cxrs form)
      (let ((cxr? (lambda (s)
		    (and (pair? (cdr s))
			 (pair? (cadr s))
			 (memq (caadr s) '(car cdr cadr cddr cdar cdddr cddddr))))))
	(and (cxr? form)
	     (let* ((arg1 (cadr form))
		    (arg2 (and arg1 (cxr? arg1) (cadr arg1)))
		    (arg3 (and arg2 (cxr? arg2) (cadr arg2))))
	       (values (string-append (hash-table-ref combinable-cxrs (car form))
				      (hash-table-ref combinable-cxrs (car arg1)) 
				      (if arg2 (hash-table-ref combinable-cxrs (car arg2)) "")
				      (if arg3 (hash-table-ref combinable-cxrs (car arg3)) ""))
		       (cadr (or arg3 arg2 arg1)))))))
#|
    ;; this builds the lists below:
    (let ((ci ())
	  (ic ()))
      (for-each
       (lambda (c)
	 (let ((name (reverse (substring (symbol->string c) 1 (- (length (symbol->string c)) 1)))))
	   (do ((sum 0)
		(len (length name))
		(i 0 (+ i 1))
		(bit 0 (+ bit 2)))
	       ((= i len) 
		(set! ci (cons (cons c sum) ci))
		(set! ic (cons (cons sum c) ic)))
	     (set! sum (+ sum (expt 2 (if (char=? (name i) #\a) bit (+ bit 1))))))))
       '(car cdr caar cadr cddr cdar caaar caadr caddr cdddr cdaar cddar cadar cdadr cadddr cddddr
	     caaaar caaadr caadar caaddr cadaar cadadr caddar cdaaar cdaadr cdadar cdaddr cddaar cddadr cdddar))
      (list (reverse ci) (reverse ic)))
|#
    (define match-cxr
      (let ((cxr->int (hash-table '(car . 1) '(cdr . 2) 
				  '(caar . 5) '(cadr . 6) '(cddr . 10) '(cdar . 9) 
				  '(caaar . 21) '(caadr . 22) '(caddr . 26) '(cdddr . 42) '(cdaar . 37) '(cddar . 41) '(cadar . 25) '(cdadr . 38) 
				  '(cadddr . 106) '(cddddr . 170) '(caaaar . 85) '(caaadr . 86) '(caadar . 89) '(caaddr . 90) '(cadaar . 101) '(cadadr . 102) 
				  '(caddar . 105) '(cdaaar . 149) '(cdaadr . 150) '(cdadar . 153) '(cdaddr . 154) '(cddaar . 165) '(cddadr . 166) '(cdddar . 169)))
	    (int->cxr (hash-table '(1 . car) '(2 . cdr) 
				  '(5 . caar) '(6 . cadr) '(10 . cddr) '(9 . cdar) 
				  '(21 . caaar) '(22 . caadr) '(26 . caddr) '(42 . cdddr) '(37 . cdaar) '(41 . cddar) '(25 . cadar) '(38 . cdadr) 
				  '(106 . cadddr) '(170 . cddddr) '(85 . caaaar) '(86 . caaadr) '(89 . caadar) '(90 . caaddr) '(101 . cadaar) '(102 . cadadr) 
				  '(105 . caddar) '(149 . cdaaar) '(150 . cdaadr) '(153 . cdadar) '(154 . cdaddr) '(165 . cddaar) '(166 . cddadr) '(169 . cdddar))))
	(lambda (c1 c2)
	  (hash-table-ref int->cxr (logand (or (hash-table-ref cxr->int c1) 0) 
					   (or (hash-table-ref cxr->int c2) 0))))))
	
    
    (define (mv-range producer env)
      (if (symbol? producer)
	  (let ((v (var-member producer env)))
	    (and (var? v)
		 (pair? ((cdr v) 'values))
		 ((cdr v) 'values)))
          (and (pair? producer)
               (if (memq (car producer) '(lambda lambda*))
                   (count-values (cddr producer))
                   (if (eq? (car producer) 'values)
                       (let ((len (- (length producer) 1)))
                         (for-each
                           (lambda (p)
                             (if (and (pair? p) (eq? (car p) 'values))
                                 (set! len (- (+ len (length p)) 2))))
                           (cdr producer))
                         (list len len))
                       (mv-range (car producer) env))))))
    
    (define (eval-constant-expression caller form)
      (if (every? code-constant? (cdr form))
	  (catch #t
	    (lambda ()
	      (let ((val (eval (copy form :readable))))
		(lint-format "perhaps ~A" caller (lists->string form val)))) ; (eq? #(0) #(0)) -> #f
	    (lambda args
	      #t))))

    (define (unbegin x)
      ((if (and (pair? x)
		(eq? (car x) 'begin))
	  cdr list)
       x))

    (define (un_{list} tree)
      (if (not (pair? tree))
	  tree
	  (if (eq? (car tree) #_{list})
	      (if (assq #_{apply_values} (cdr tree))
		  (if (and (pair? (cadr tree))
			   (eq? (caadr tree) #_{apply_values}))
		      `(append ,(cadadr tree) ,(cadr (caddr tree)))
		      `(cons ,(cadr tree) ,(cadr (caddr tree))))
		  (cons 'list (un_{list} (cdr tree))))
	      (cons (if (eq? (car tree) #_{append}) 
			'append
			(un_{list} (car tree)))
		    (un_{list} (cdr tree))))))

    (define (qq-tree? tree)
      (and (pair? tree)
	   (or (eq? (car tree) #_{apply_values})
	       (if (and (eq? (car tree) #_{list})
			(assq #_{apply_values} (cdr tree)))
		   (or (not (= (length tree) 3))
		       (not (and (pair? (caddr tree))
				 (eq? (caaddr tree) #_{apply_values})))
		       (qq-tree? (cadr (caddr tree)))
		       (if (and (pair? (cadr tree))
				(eq? (caadr tree) #_{apply_values}))
			   (qq-tree? (cadadr tree))
			   (qq-tree? (cadr tree))))
		   (or (qq-tree? (car tree))
		       (qq-tree? (cdr tree)))))))

    (define special-case-functions
      (let ((h (make-hash-table)))
	
	;; ---------------- member and assoc ----------------
	(let ()
	  (define (sp-memx caller head form env)
	    (define (list-one? p)
	      (and (pair? p)
		   (pair? (cdr p))
		   (null? (cddr p))
		   (case (car p)
		     ((list) cadr)
		     ((quote)
		      (and (pair? (cadr p))
			   (null? (cdadr p))
			   (if (symbol? (caadr p))
			       (lambda (x)
				 (list 'quote (caadr x)))
			       caadr)))
		     (else #f))))
	  
	    (when (= (length form) 4)
	      (let ((func (list-ref form 3)))
		(if (symbol? func)
		    (if (memq func '(eq? eqv? equal?))   ; (member x y eq?) -> (memq x y)
			(let ((op (if (eq? head 'member) ; (member (car x) entries equal?) -> (member (car x) entries)
				      (case func ((eq?) 'memq) ((eqv?) 'memv) (else 'member))
				      (case func ((eq?) 'assq) ((eqv?) 'assv) (else 'assoc)))))
			  (lint-format "perhaps ~A" caller (lists->string form `(,op ,(cadr form) ,(caddr form)))))
			(let ((sig (procedure-signature (symbol->value func)))) ; arg-signature here is too cranky
			  (if (and (pair? sig)
				   (not (eq? 'boolean? (car sig)))
				   (not (and (pair? (car sig))
					     (memq 'boolean? (car sig)))))
			      (lint-format "~A is a questionable ~A function" caller func head)))) ; (member 1 x abs)
		    ;; func not a symbol
		    (if (and (pair? func)
			     (= (length func) 3)       ; (member 'a x (lambda (a b c) (eq? a b)))
			     (eq? (car func) 'lambda)
			     (pair? (cadr func))
			     (pair? (caddr func)))
			(if (not (memv (length (cadr func)) '(2 -1)))
			    (lint-format "~A equality function (optional third arg) should take two arguments" caller head)
			    (if (eq? head 'member)
				(let ((eq (caddr func))
				      (args (cadr func)))
				  (if (and (memq (car eq) '(eq? eqv? equal?))
					   (eq? (car args) (cadr eq))
					   (pair? (caddr eq))
					   (eq? (car (caddr eq)) 'car)
					   (pair? (cdr (caddr eq)))
					   (pair? (cdr args))
					   (eq? (cadr args) (cadr (caddr eq))))
				      (lint-format "member might perhaps be ~A" ; (member 'a x (lambda (a b) (eq? a (car b))))
						   caller
						   (if (or (eq? func 'eq?)
							   (eq? (car (caddr func)) 'eq?))
						       'assq
						       (if (eq? (car (caddr func)) 'eqv?) 
							   'assv 
							   'assoc)))))))))))
	    
	    (when (= (length form) 3)
	      (let ((selector (cadr form))
		    (items (caddr form)))
		
		(let ((current-eqf (case head ((memq assq) 'eq?) ((memv assv) 'eqv?) (else 'equal?)))
		      (selector-eqf (car (eqf selector env)))
		      (one-item (and (memq head '(memq memv member)) (list-one? items))))
		  ;; one-item assoc doesn't simplify cleanly
		  
		  (if one-item
		      (let* ((target (one-item items))
			     (iter-eqf (eqf target env)))
			(if (or (symbol? target)
				(and (pair? target)
				     (not (eq? (car target) 'quote))))
			    (set! target (list 'quote target))) ; ; (member x (list "asdf")) -> (string=? x "asdf") -- maybe equal? here?
			(lint-format "perhaps ~A" caller (lists->string form `(,(cadr iter-eqf) ,selector ,target))))
		      
		      ;; not one-item
		      (letrec ((duplicates? (lambda (lst fnc)
					      (and (pair? lst)
						   (or (fnc (car lst) (cdr lst))
						       (duplicates? (cdr lst) fnc)))))
			       (duplicate-constants? (lambda (lst fnc)
						       (and (pair? lst)
							    (or (and (constant? (car lst))
								     (fnc (car lst) (cdr lst)))
								(duplicate-constants? (cdr lst) fnc))))))
			(if (and (symbol? selector-eqf)   ; (memq 1.0 x): perhaps memq -> memv
				 (not (eq? selector-eqf current-eqf)))
			    (lint-format "~A: perhaps ~A -> ~A" caller (truncated-list->string form) head 
					 (if (memq head '(memq memv member))
					     (case selector-eqf ((eq?) 'memq) ((eqv?) 'memv) ((equal?) 'member))
					     (case selector-eqf ((eq?) 'assq) ((eqv?) 'assv) ((equal?) 'assoc)))))

			;; --------------------------------
			;; check for head mismatch with items
			(when (pair? items)
			  (when (or (eq? (car items) 'list)
				    (and (eq? (car items) 'quote)
					 (pair? (cadr items))))
			    (let ((elements ((if (eq? (car items) 'quote) cadr cdr) items)))
			      (let ((baddy #f))
				(catch #t 
				  (lambda () 
				    (set! baddy ((if (eq? (car items) 'list) duplicate-constants? duplicates?)
						 elements (symbol->value head))))
				  (lambda args #f))
				(if (pair? baddy) ; (member x (list "asd" "abc" "asd"))
				    (lint-format "duplicated entry ~S in ~A" caller (car baddy) items)))

			      (when (proper-list? elements)
				(let ((maxf #f)
				      (keys (if (eq? (car items) 'quote)
						(if (memq head '(memq memv member))
						    elements 
						    (and (every? pair? elements)
							 (map car elements)))
						(if (memq head '(memq memv member))
						    (and (every? code-constant? elements)
							 elements)
						    (and (every? (lambda (e)
								   (and (pair? e)
									(eq? (car e) 'quote)))
								 elements)
							 (map caadr elements))))))
				  (when (proper-list? keys)
				    (if (eq? (car items) 'quote)
					(do ((p keys (cdr p)))
					    ((or (null? p)
						 (memq maxf '(equal? #t))))
					  (let ((element (car p)))
					    (if (symbol? element)
						(if (not maxf)
						    (set! maxf 'eq?))
						(if (pair? element)
						    (begin
						      (if (and (eq? (car element) 'quote)
							       (pair? (cdr element)))
							  (lint-format "stray quote? ~A" caller form)) ; (memq x '(a 'b c))
						      (set! maxf #t))
						    (let ((type (if (symbol? element)
								    'eq?
								    (car (->eqf (->simple-type element))))))
						      (if (or (memq maxf '(#f eq?))
							      (memq type '(#t equal?)))
							  (set! maxf type)))))))
					;; else (list ...)
					(do ((p keys (cdr p)))
					    ((or (null? p)
						 (memq maxf '(equal? #t))))
					  (let ((element (car p)))
					    (if (symbol? element)
						(set! maxf #t)
						(let ((type (car (eqf element env))))
						  (if (or (memq maxf '(#f eq?))
							  (memq type '(#t equal?)))
						      (set! maxf type)))))))
				    (case maxf
				      ((eq?)
				       (if (not (memq head '(memq assq))) ; (member (car op) '(x y z))
					   (lint-format "~A could be ~A in ~A" caller 
							head 
							(if (memq head '(memv member)) 'memq 'assq)
							form)))
				      ((eqv?)
				       (if (not (memq head '(memv assv))) ; (memq (strname 0) '(#\{ #\[ #\()))
					   (lint-format "~A ~Aould be ~A in ~A" caller 
							head 
							(if (memq head '(memq assq)) "sh" "c")
							(if (memq head '(memq member)) 'memv 'assv)
							form)))
				      ((equal? #t)                        ; (memq (car op) '("a" #()))
				       (if (not (memq head '(member assoc)))
					   (lint-format "~A should be ~A in ~A" caller 
							head 
							(if (memq head '(memq memv)) 'member 'assoc)
							form)))))))
			      ;; --------------------------------

			      (if (and (= (length elements) 2)  ; (memq expr '(#t #f))
				       (memq #t elements)
				       (memq #f elements))
				  (lint-format "perhaps ~A" caller (lists->string form `(boolean? ,selector))))))
			  ;; not (memv x '(0 0.0)) -> (zero? x) because x might not be a number

			  (let ((memx (memq head '(memq memv member))))
			    (case (car items)
			      ((map)
			       (when (and memx (= (length items) 3))
				 (let ((mapf (cadr items))
				       (map-items (caddr items)))
				   (cond ((eq? mapf 'car)         ; (memq x (map car y)) -> (assq x y)
					  (lint-format "perhaps use assoc: ~A" caller
						       (lists->string form `(,(case current-eqf ((eq?) 'assq) ((eqv?) 'assv) ((equal?) 'assoc)) 
									     ,selector ,map-items))))

					 ((eq? selector #t)
					  (if (eq? mapf 'null?)   ; (memq #t (map null? items)) -> (memq () items)
					      (lint-format "perhaps ~A" caller 
							   (lists->string form `(memq () ,map-items)))
					      (let ((b (if (eq? mapf 'b) 'c 'b))) 
						;; (memq #t (map cadr items)) -> (member #t items (lambda (a b) (cadr b)))
						(lint-format "perhaps avoid 'map: ~A" caller 
							     (lists->string form `(member #t ,map-items (lambda (a ,b) (,mapf ,b))))))))
					 
					 ((and (pair? selector)
					       (eq? (car selector) 'string->symbol) ; this could be extended, but it doesn't happen
					       (eq? mapf 'string->symbol)
					       (not (and (pair? map-items)
							 (eq? (car map-items) 'quote))))
					  (lint-format "perhaps ~A" caller  
						       ;; (memq (string->symbol x) (map string->symbol y)) -> (member x y string=?)
						       (lists->string form `(member ,(cadr selector) ,map-items string=?))))

					 (else            
					  ;; (member x (map b items)) -> (member x items (lambda (a c) (equal? a (b c))))
					  (let ((b (if (eq? mapf 'b) 'c 'b))) ; a below can't collide because eqf won't return 'a
					    (lint-format "perhaps avoid 'map: ~A" caller 
							 (lists->string form `(member ,selector ,map-items 
										      (lambda (a ,b) (,current-eqf a (,mapf ,b))))))))))))

			      ((string->list)             ; (memv c (string->list s)) -> (char-position c s)
			       (lint-format "perhaps ~A" caller 
					    (lists->string form `(char-position ,(cadr form) ,@(cdr items)))))

			      ((cons)                     ; (member x (cons y z)) -> (or (equal? x y) (member x z))
			       (if (not (pair? selector))
				   (lint-format "perhaps avoid 'cons: ~A" caller
						(lists->string form `(or (,current-eqf ,selector ,(cadr items))
									 (,head ,selector ,(caddr items)))))))

			      ((append)                   ; (member x (append (list x) y)) -> (or (equal? x x) (member x y))
			       (if (and (not (pair? selector))
					(= (length items) 3)
					(pair? (cadr items))
					(eq? (caadr items) 'list)
					(null? (cddadr items)))
				   (lint-format "perhaps ~A" caller
						(lists->string form `(or (,current-eqf ,selector ,(cadadr items))
									 (,head ,selector ,(caddr items)))))))))))))
		(when (and (memq head '(memq memv))
			   (pair? items)
			   (eq? (car items) 'quote)
			   (pair? (cadr items)))
		  (let ((nitems (length (cadr items))))

		    (if (pair? selector)                        ; (memv (string-ref x 0) '(+ -)) -> #f etc
			(let ((sig (arg-signature (car selector) env)))
			  (if (and (pair? sig)
				   (symbol? (car sig))
				   (not (eq? (car sig) 'values)))
			      (let ((vals (map (lambda (item)
						 (if ((symbol->value (car sig)) item) item (values)))
					       (cadr items))))
				(if (not (= (length vals) nitems))
				    (lint-format "perhaps ~A" caller
						 (lists->string form
								(and (pair? vals)
								     `(,head ,selector ',vals)))))))))
		    (if (> nitems 20)
			(lint-format "perhaps use a hash-table here, rather than ~A" caller (truncated-list->string form)))
		    
		    (let ((bad (find-if (lambda (x)
					  (not (or (symbol? x)
						   (char? x)
						   (number? x)
						   (procedure? x) ; (memq abs '(1 #_abs 2)) !
						   (memq x '(#f #t () #<unspecified> #<undefined> #<eof>)))))
					(cadr items))))
		      (if bad
			  (lint-format (if (and (pair? bad)
						(eq? (car bad) 'unquote))
					   (values "stray comma? ~A" caller)  ; (memq x '(a (unquote b) c))
					   (values "pointless list member: ~S in ~A" caller bad))
				       ;; quoted item here is caught above    ; (memq x '(a (+ 1 2) 3))
				       form))))))))  

	  (for-each (lambda (f)
		      (hash-table-set! h f sp-memx))
		    '(memq assq memv assv member assoc)))
	
	;; ---------------- car, cdr, etc ----------------
	(let ()
	  (define (sp-crx caller head form env)
	    (if (not (= line-number last-simplify-cxr-line-number))
		((lambda* (cxr arg)
		   (when cxr
		     (set! last-simplify-cxr-line-number line-number)
		     (cond ((< (length cxr) 5)                           ; (car (cddr x)) -> (caddr x)
			    (lint-format "perhaps ~A" caller 
					 (lists->string form `(,(symbol "c" cxr "r") ,arg))))
			   
			   ;; if it's car|cdr followed by cdr's, use list-ref|tail
			   ((not (char-position #\a cxr))                ; (cddddr (cddr x)) -> (list-tail x 6)
			    (lint-format "perhaps ~A" caller (lists->string form `(list-tail ,arg ,(length cxr)))))
			   
			   ((not (char-position #\a (substring cxr 1)))  ; (car (cddddr (cddr x))) -> (list-ref x 6)
			    (lint-format "perhaps ~A" caller (lists->string form `(list-ref ,arg ,(- (length cxr) 1)))))
			   
			   (else (set! last-simplify-cxr-line-number -1)))))
		 (combine-cxrs form)))
	    
	    (when (pair? (cadr form))
	      (let ((arg (cadr form)))
		(when (eq? head 'car)                             
		  (case (car arg) 
		    ((list-tail)                   ; (car (list-tail x y)) -> (list-ref x y)
		     (lint-format "perhaps ~A" caller (lists->string form `(list-ref ,(cadr arg) ,(caddr arg)))))

		    ((memq memv member assq assv assoc)
		     (if (pair? (cdr arg))         ; (car (memq x ...)) is either x or (car #f) -> error
			 (lint-format "~A is ~A, or an error" caller (truncated-list->string form) (cadr arg))))))
		
		(when (and (eq? (car arg) 'or) ; (cdr (or (assoc x y) (cons 1 2))) -> (cond ((assoc x y) => cdr) (else 2))
			   (not (eq? form last-rewritten-internal-define))
			   (= (length arg) 3))
		  (let ((arg1 (cadr arg))
			(arg2 (caddr arg)))
		    (if (and (pair? arg2)
			     (or (and (memq (car arg2) '(cons list #_{list}))
				      (eq? head 'cdr))
				 (memq (car arg2) '(error throw))
				 (and (eq? (car arg2) 'quote)
				      (pair? (cdr arg2))
				      (pair? (cadr arg2)))))
			(lint-format "perhaps ~A" caller
				     (lists->string form ; (cdr (or (assoc n oi) (list n y))) -> (cond ((assoc n oi) => cdr) (else (list y)))
						    `(cond (,arg1 => ,head)
							   (else ,(case (car arg2)
								    ((quote) ((symbol->value head) (cadr arg2)))
								    ((cons) (caddr arg2))
								    ((error throw) arg2)
								    (else `(list ,@(cddr arg2)))))))))))
		(if (and (memq head '(car cdr))
			 (eq? (car arg) 'cons))
		    (lint-format "(~A~A) is the same as ~A"             ; (car(cons 1 2)) is the same as 1
				 caller head
				 (truncated-list->string arg)
				 (if (eq? head 'car)
				     (truncated-list->string (cadr arg))
				     (truncated-list->string (caddr arg)))))
		
		(when (memq head '(car cadr caddr cadddr))
		  (if (memq (car arg) '(string->list vector->list))    ; (car (string->list x)) -> (string-ref x 0)
		      (lint-format "perhaps ~A" caller (lists->string form 
								      `(,(if (eq? (car arg) 'string->list) 'string-ref 'vector-ref)
									,(cadr arg) 
									,(case head ((car) 0) ((cadr) 1) ((caddr) 2) (else 3)))))
		      (if (memq (car arg) '(reverse reverse!))
			  (lint-format "perhaps ~A~A" caller           ; (car (reverse x)) -> (list-ref x (- (length x) 1))
				       (if (eq? head 'car)
					   "use 'last from srfi-1, or "
					   "")
				       (lists->string form
						      (if (symbol? (cadr arg))
							  `(list-ref ,(cadr arg) 
								     (- (length ,(cadr arg)) 
									,(case head ((car) 1) ((cadr) 2) ((caddr) 3) (else 4))))
							  `(let ((_1_ ,(cadr arg)))  ; let is almost certainly cheaper than reverse
							     (list-ref _1_ (- (length _1_)
									      ,(case head ((car) 1) ((cadr) 2) ((caddr) 3) (else 4))))))))))))))
	  (for-each (lambda (f)
		      (hash-table-set! h (car f) sp-crx))
		    combinable-cxrs))
	
	;; ---------------- set-car! ----------------
	(let ()
	  (define (sp-set-car! caller head form env)
	    (when (= (length form) 3)
	      (let ((target (cadr form)))
		(if (pair? target)
		    (case (car target)
		      
		      ((list-tail)                              ; (set-car! (list-tail x y) z) -> (list-set! x y z)
		       (lint-format "perhaps ~A" caller (lists->string form `(list-set! ,(cadr target) ,(caddr target) ,(caddr form)))))
		      
		      ((cdr cddr cdddr cddddr)                  ; (set-car! (cddr (cdddr x)) y) -> (list-set! x 5 y)
		       (set! last-simplify-cxr-line-number line-number)
		       (lint-format "perhaps ~A" caller       
				    (lists->string form 
						   (if (and (pair? (cadr target))
							    (memq (caadr target) '(cdr cddr cdddr cddddr)))
						       ;; (set-car! (cdr (cddr x)) y) -> (list-set! x 3 y)
						       `(list-set! ,(cadadr target)
								   ,(+ (cdr-count (car target)) (cdr-count (caadr target))) 
								   ,(caddr form))
						       ;; (set-car! (cdr x) y) -> (list-set! x 1 y)
						       `(list-set! ,(cadr target) 
								   ,(cdr-count (car target)) 
								   ,(caddr form)))))))))))
	  (hash-table-set! h 'set-car! sp-set-car!))

	;; ---------------- not ----------------
	(let ()
	  (define (sp-not caller head form env)
	    (if (and (pair? (cdr form))
		     (pair? (cadr form)))
		(if (eq? (caadr form) 'not)
		    (let ((str (truncated-list->string (cadadr form)))) ; (not (not x)) -> (and x #t)
		      (lint-format "if you want a boolean, (not (not ~A)) -> (and ~A #t)" 'paranoia str str))
		    (let ((sig (arg-signature (caadr form) env)))
		      (if (and (pair? sig)
			       (if (pair? (car sig))                    ; (not (+ x y))
				   (not (memq 'boolean? (car sig)))
				   (not (memq (car sig) '(#t values boolean?)))))
			  (lint-format "~A can't be true (~A never returns #f)" caller (truncated-list->string form) (caadr form))))))
					 
	    (if (not (= line-number last-simplify-boolean-line-number))
		(let ((val (simplify-boolean form () () env)))
		  (set! last-simplify-boolean-line-number line-number)
		  (if (not (equal? form val))                           ;  (not (and (> x 2) (not z))) -> (or (<= x 2) z)
		      (lint-format "perhaps ~A" caller (lists->string form val))))))
	  
	  (hash-table-set! h 'not sp-not))
	
	;; ---------------- and/or ----------------
	(let ()
	  (define (sp-and caller head form env)
	    (if (not (= line-number last-simplify-boolean-line-number))
		(let ((val (simplify-boolean form () () env)))
		  (set! last-simplify-boolean-line-number line-number)
		  (if (not (equal? form val))          ; (and (not x) (not y)) -> (not (or x y))
		      (lint-format "perhaps ~A" caller (lists->string form val)))))
	    (if (pair? (cdr form))
		(do ((p (cdr form) (cdr p)))
		    ((null? (cdr p)))
		  (if (and (pair? (car p))
			   (eq? (caar p) 'if)
			   (= (length (car p)) 3))     ; (and (member n cvars) (if (pair? open) (not (member n open))) (not (eq? n open)))
		      (lint-format "one-armed if might cause confusion here: ~A" caller form)))))
	  (hash-table-set! h 'and sp-and)
	  (hash-table-set! h 'or sp-and))
	
	;; ---------------- = ----------------
	(let ()
	 (define (sp-= caller head form env)
	   (let ((len (length form)))
	     (if (and (> len 2)
		      (let any-real? ((lst (cdr form))) ; ignore 0.0 and 1.0 in this since they normally work
			(and (pair? lst)
			     (or (and (number? (car lst))
				      (not (rational? (car lst)))
				      (not (member (car lst) '(0.0 1.0) =)))
				 (any-real? (cdr lst))))))    ; (= x 1.5)
		 (lint-format "= can be troublesome with floats: ~A" caller (truncated-list->string form)))
	     
	     (let ((cleared-form (cons = (remove-if (lambda (x) (not (number? x))) (cdr form)))))
	       (if (and (> (length cleared-form) 2)
			(not (checked-eval cleared-form)))    ; (= 1 y 2)
		   (lint-format "this comparison can't be true: ~A" caller (truncated-list->string form))))
	     
	     (when (= len 3)
	       (let ((arg1 (cadr form))
		     (arg2 (caddr form)))
		 ;; (= (+ x a) (+ y a)) and various equivalents happen very rarely (only in test suites it appears)
		 (let ((var (or (and (memv arg1 '(0 1))
				     (pair? arg2)
				     (eq? (car arg2) 'length)
				     (cadr arg2))
				(and (memv arg2 '(0 1))
				     (pair? arg1)
				     (eq? (car arg1) 'length)
				     (cadr arg1)))))
		   ;; we never seem to have var-member/initial-value/history here to distinguish types
		   ;;   and a serious attempt to do so was a bust.
		   (if var
		       (if (or (eqv? arg1 0)    ;  (= (length x) 0) -> (null? x)
			       (eqv? arg2 0))
			   (lint-format "perhaps (assuming ~A is a list), ~A" caller var 
					(lists->string form `(null? ,var)))
			   (if (symbol? var)    ;  (= (length x) 1) -> (and (pair? x) (null? (cdr x)))
			       (lint-format "perhaps (assuming ~A is a list), ~A" caller var 
					    (lists->string form `(and (pair? ,var) (null? (cdr ,var))))))))))
	       (unrelop caller '= form))
	     (check-char-cmp caller '= form)))
	 (hash-table-set! h '= sp-=))
	
	;; ---------------- < > <= >= ----------------
	(let ()
	  (define (sp-< caller head form env)
	    (let ((cleared-form (cons head ; keep operator
				      (remove-if (lambda (x) 
						   (not (number? x))) 
						 (cdr form)))))
	      (if (and (> (length cleared-form) 2)
		       (not (checked-eval cleared-form)))  ; (< x 1 2 0 y)
		  (lint-format "this comparison can't be true: ~A" caller (truncated-list->string form))))
	    
	    (if (= (length form) 3)
		(unrelop caller head form)
		(when (> (length form) 3)
		  (if (and (memq head '(< >))              ; (< x y x) -> #f
			   (repeated-member? (cdr form) env))
		      (lint-format "perhaps ~A" caller (truncated-lists->string form #f))
		      (if (and (memq head '(<= >=))
			       (repeated-member? (cdr form) env))
			  (do ((last-arg (cadr form))
			       (new-args (list (cadr form)))
			       (lst (cddr form) (cdr lst)))
			      ((null? lst) 
			       (if (repeated-member? new-args env)       ; (<= x y x z x) -> (= x y z)
				   (lint-format "perhaps ~A" caller (truncated-lists->string form `(= ,@(lint-remove-duplicates (reverse new-args) env))))
				   (if (< (length new-args) (length (cdr form)))
				       (lint-format "perhaps ~A" caller  ; (<= x x y z) -> (= x y z)
						    (truncated-lists->string form (or (null? (cdr new-args))
										      `(= ,@(reverse new-args))))))))
			    (unless (equal? (car lst) last-arg)
			      (set! last-arg (car lst))
			      (set! new-args (cons last-arg new-args))))))))
	    
	    (cond ((not (= (length form) 3)))

		  ((and (real? (cadr form))
			(or (< (cadr form) 0)
			    (and (zero? (cadr form))
				 (eq? head '>)))
			(pair? (caddr form))   ; (> 0 (string-length x))
			(hash-table-ref non-negative-ops (caaddr form)))
		   (lint-format "~A can't be negative: ~A" caller (caaddr form) (truncated-list->string form)))
		  
		  ((and (real? (caddr form))
			(or (< (caddr form) 0)
			    (and (zero? (caddr form))
				 (eq? head '<)))
			(pair? (cadr form))    ; (< (string-length x) 0)
			(hash-table-ref non-negative-ops (caadr form)))
		   (lint-format "~A can't be negative: ~A" caller (caadr form) (truncated-list->string form)))
		  
		  ((and (pair? (cadr form))
			(eq? (caadr form) 'length))
		   (let ((arg (cadadr form)))
		     (when (symbol? arg)       ;  (>= (length x) 0) -> (list? x)
		       ;; see comment above about distinguishing types!  (twice I've wasted my time)
		       (if (eqv? (caddr form) 0)
			   (lint-format "perhaps~A ~A" caller
					(if (eq? head '<) "" (format #f " (assuming ~A is a proper list)," arg))
					(lists->string form
						       (case head
							 ((<)  `(and (pair? ,arg) (not (proper-list? ,arg))))
							 ((<=) `(null? ,arg))
							 ((>)  `(pair? ,arg))
							 ((>=) `(list? ,arg)))))
			   (if (and (eqv? (caddr form) 1)
				    (not (eq? head '>)))  ; (<= (length x) 1) -> (or (null? x) (null? (cdr x)))
			       (lint-format "perhaps (assuming ~A is a proper list), ~A" caller arg
					    (lists->string form
							   (case head
							     ((<)  `(null? ,arg))
							     ((<=) `(or (null? ,arg) (null? (cdr ,arg))))
							     ((>)  `(and (pair? ,arg) (pair? (cdr ,arg))))
							     ((>=) `(pair? ,arg))))))))))
		  ((and (pair? (caddr form))
			(eq? (caaddr form) 'length))
		   (let ((arg (cadr (caddr form))))
		     (when (symbol? arg)                  ;  (>= 0 (length x)) -> (null? x)
		       (if (eqv? (cadr form) 0)
			   (lint-format "perhaps~A ~A" caller
					(if (eq? head '>) "" (format #f " (assuming ~A is a proper list)," arg))
					(lists->string form
						       (case head
							 ((<)  `(pair? ,arg))
							 ((<=) `(list? ,arg))
							 ((>)  `(and (pair? ,arg) (not (proper-list? ,arg))))
							 ((>=) `(null? ,arg)))))
			   (if (and (eqv? (cadr form) 1)
				    (not (eq? head '<))) ; (> 1 (length x)) -> (null? x)
			       (lint-format "perhaps (assuming ~A is a proper list), ~A" caller arg
					    (lists->string form
							   (case head
							     ((<)  `(and (pair? ,arg) (pair? (cdr ,arg))))
							     ((<=) `(pair? ,arg))
							     ((>)  `(null? ,arg))
							     ((>=) `(or (null? ,arg) (null? (cdr ,arg))))))))))))
		  ((and (eq? head '<)
			(eqv? (caddr form) 1)
			(pair? (cadr form))           ; (< (vector-length x) 1) -> (equal? x #())
			(memq (caadr form) '(string-length vector-length)))
		   (lint-format "perhaps ~A" caller (lists->string form `(,(if (eq? (caadr form) 'string-length) 'string=? 'equal?)
									  ,(cadadr form)
									  ,(if (eq? (caadr form) 'string-length) "" #())))))
		  ((and (eq? head '>)
			(eqv? (cadr form) 1)
			(pair? (caddr form))          ; (> 1 (string-length x)) -> (string=? x "")
			(memq (caaddr form) '(string-length vector-length)))
		   (lint-format "perhaps ~A" caller (lists->string form `(,(if (eq? (caaddr form) 'string-length) 'string=? 'equal?)
									  ,(cadr (caddr form))
									  ,(if (eq? (caaddr form) 'string-length) "" #())))))
		  ((and (memq head '(<= >=))
			(or (and (eqv? (caddr form) 0)
				 (pair? (cadr form))  ; (<= (string-length m) 0) -> (= (string-length m) 0)
				 (hash-table-ref non-negative-ops (caadr form)))
			    (and (eqv? (cadr form) 0)
				 (pair? (caddr form))
				 (hash-table-ref non-negative-ops (caaddr form)))))
		   (lint-format "~A is never negative, so ~A" caller
				((if (eqv? (caddr form) 0) caadr caaddr) form)
				(lists->string form (or (not (eq? (eq? head '<=) 
								  (eqv? (caddr form) 0)))
							`(= ,@(cdr form))))))
		  ((and (eqv? (caddr form) 256)
			(pair? (cadr form))          ; (< (char->integer key) 256) -> #t
			(eq? (caadr form) 'char->integer))
		   (lint-format "perhaps ~A" caller
				(lists->string form (and (memq head '(< <=)) #t))))
		  
		  ((or (and (eqv? (cadr form) 0)     ; (> (numerator x) 0) -> (> x 0)
			    (pair? (caddr form))
			    (eq? (caaddr form) 'numerator))
		       (and (eqv? (caddr form) 0)
			    (pair? (cadr form))
			    (eq? (caadr form) 'numerator)))
		   (lint-format "perhaps ~A" caller
				(lists->string form
					       (if (eqv? (cadr form) 0)
						   `(,head ,(cadr form) ,(cadr (caddr form)))
						   `(,head ,(cadadr form) ,(caddr form)))))))
	    (check-char-cmp caller head form))
	  ;; could change (> x 0) to (positive? x) and so on, but the former is clear and ubiquitous
	  
	  (for-each (lambda (f)
		      (hash-table-set! h f sp-<))
		    '(< > <= >=))) ; '= handled above
	
	;; ---------------- char< char> etc ----------------
	(let ()
	  (define (sp-char< caller head form env)
	    ;; only once: (char<=? #\0 c #\1)
	    (let ((cleared-form (cons head ; keep operator
				      (remove-if (lambda (x) 
						   (not (char? x))) 
						 (cdr form)))))
	      (if (and (> (length cleared-form) 2)   ; (char>? x #\a #\b y)
		       (not (checked-eval cleared-form)))
		  (lint-format "this comparison can't be true: ~A" caller (truncated-list->string form))))
	    (if (and (eq? head 'char-ci=?)           ; (char-ci=? x #\return)
		     (pair? (cdr form))
		     (pair? (cddr form))
		     (null? (cdddr form))            ; (char-ci=? x #\return)
		     (or (and (char? (cadr form))
			      (char=? (cadr form) (other-case (cadr form))))
			 (and (char? (caddr form))
			      (char=? (caddr form) (other-case (caddr form))))))
		(lint-format "char-ci=? could be char=? here: ~A" caller form)
		
		(when (and (eq? head 'char=?)        ; (char=? #\a (char-downcase x)) -> (char-ci=? #\a x)
			   (let ((casef (let ((op #f))
					  (lambda (a)
					    (or (char? a)
						(and (pair? a)
						     (memq (car a) '(char-downcase char-upcase))
						     (if op
							 (eq? op (car a))
							 (set! op (car a)))))))))
			     (every? casef (cdr form))))
		  (lint-format "perhaps ~A" caller
			       (lists->string form   ; (char=? #\a (char-downcase x)) -> (char-ci=? #\a x)
					      `(char-ci=? ,@(map (lambda (a)
								   (if (and (pair? a)
									    (memq (car a) '(char-upcase char-downcase)))
								       (cadr a)
								       a))
								   (cdr form))))))))
	  (for-each (lambda (f)
		      (hash-table-set! h f sp-char<))
		    '(char<? char>? char<=? char>=? char=? char-ci<? char-ci>? char-ci<=? char-ci>=? char-ci=?)))
	
	
	;; ---------------- string< string> etc ----------------
	(let ()
	  (define (sp-string< caller head form env)
	    (let ((cleared-form (cons head ; keep operator
				      (remove-if (lambda (x) 
						   (not (string? x))) 
						 (cdr form)))))
	      (if (and (> (length cleared-form) 2)  ; (string>? "a" x "b" y)
		       (not (checked-eval cleared-form)))
		  (lint-format "this comparison can't be true: ~A" caller (truncated-list->string form))))

	    (if (and (> (length form) 2)
		     (every? (let ((op #f))         ; (string=? x (string-downcase y)) -> (string-ci=? x y)
			       (lambda (a)
				 (and (pair? a)
				      (memq (car a) '(string-downcase string-upcase))
				      (if op
					  (eq? op (car a))
					  (set! op (car a))))))
			     (cdr form)))
		(lint-format "perhaps ~A" caller    ; (string=? (string-downcase x) (string-downcase y)) -> (string-ci=? x y)
			     (lists->string form
					    (let ((op (case head
							((string=?) 'string-ci=?)
							((string<=?) 'string-ci<=?)
							((string>=?) 'string-ci>=?)
							((string<?) 'string-ci<?)
							((string>?) 'string-ci>?)
							(else head))))
					      `(,op ,@(map (lambda (a)
							     (if (and (pair? a)
								      (memq (car a) '(string-upcase string-downcase)))
								 (cadr a)
								 a))
							   (cdr form)))))))

	    (if (any? (lambda (a) ; string-copy is redundant in arg list
			(and (pair? a)
			     (memq (car a) '(copy string-copy))
			     (null? (cddr a))))
		      (cdr form))
		(let cleaner ((args (cdr form)) (new-args ())) ; (string=? "" (string-copy "")) -> (string=? "" "")
		  (if (not (pair? args))
		      (lint-format "perhaps ~A" caller (lists->string form `(,head ,@(reverse new-args))))
		      (let ((a (car args)))
			(cleaner (cdr args)
				 (cons (if (and (pair? a)
						(memq (car a) '(copy string-copy))
						(null? (cddr a)))
					   (cadr a)
					   a)
				       new-args))))))

	    (when (and (eq? head 'string=?)
		       (= (length form) 3))                ; (string=? (symbol->string a) (symbol->string b)) -> (eq? a b)
	      (if (and (pair? (cadr form))
		       (eq? (caadr form) 'symbol->string)
		       (pair? (caddr form))
		       (eq? (caaddr form) 'symbol->string))
		  (lint-format "perhaps ~A" caller (lists->string form `(eq? ,(cadadr form) ,(cadr (caddr form)))))
		  (let ((s1 #f)
			(s2 #f))
		    (if (and (string? (cadr form))
			     (= (length (cadr form)) 1))
			(begin
			  (set! s1 (cadr form))
			  (set! s2 (caddr form)))
			(if (and (string? (caddr form))
				 (= (length (caddr form)) 1))
			    (begin
			      (set! s1 (caddr form))
			      (set! s2 (cadr form)))))
		    (if (and s1                         ; (string=? (substring r 0 1) "S")
			     (pair? s2)
			     (eq? (car s2) 'substring)       
			     (= (length s2) 4)
			     (eqv? (list-ref s2 2) 0)
			     (eqv? (list-ref s2 3) 1))
			(lint-format "perhaps ~A" caller
				     (lists->string form `(char=? (string-ref ,(cadr s2) 0) ,(string-ref s1 0))))))))
	    
	    (if (every? (lambda (a)                     ; (string=? "#" (string (string-ref s 0))) -> (char=? #\# (string-ref s 0))
			  (or (and (string? a)
				   (= (length a) 1))
			      (and (pair? a)
				   (eq? (car a) 'string))))
			(cdr form))
		(lint-format "perhaps ~A" caller
			     (lists->string form
					    `(,(symbol "char" (substring (symbol->string head) 6))
					      ,@(map (lambda (a)
						       (if (string? a)
							   (string-ref a 0)
							   (cadr a)))
						     (cdr form)))))))
	  (for-each (lambda (f)
		      (hash-table-set! h f sp-string<))
		    '(string<? string>? string<=? string>=? string=? string-ci<? string-ci>? string-ci<=? string-ci>=? string-ci=?)))
	
	;; ---------------- length ----------------
	(let ()
	 (define (sp-length caller head form env)
	   (when (pair? (cdr form))
	     (if (pair? (cadr form))
		 (let ((arg (cadr form)))
		   (case (car arg)
		     ((string->list vector->list)
		      (if (null? (cddr arg))           ; string->list has start:end etc ; (length (string->list x)) -> (length x)
			  (lint-format "perhaps ~A" caller (lists->string form `(length ,(cadr arg))))
			  (if (pair? (cdddr arg))
			      (if (and (integer? (cadddr arg))  ; (length (vector->list x 1)) -> (- (length x) 1)
				       (integer? (caddr arg)))
				  (lint-format "perhaps ~A -> ~A" caller (truncated-list->string form) (max 0 (- (cadddr arg) (caddr arg))))
				  (lint-format "perhaps ~A" caller (lists->string form `(- ,(cadddr arg) ,(caddr arg)))))
			      (lint-format "perhaps ~A" caller (lists->string form `(- (length ,(cadr arg)) ,(caddr arg)))))))
		     
		     ((reverse reverse! list->vector list->string let->list)
		      (lint-format "perhaps ~A" caller (lists->string form `(length ,(cadr arg)))))
		     
		     ((cons)                          ; (length (cons item items)) -> (+ (length items) 1)
		      (lint-format "perhaps ~A" caller (lists->string form `(+ (length ,(caddr arg)) 1))))
		     
		     ((make-list)                     ; (length (make-list 3)) -> 3
		      (lint-format "perhaps ~A" caller (lists->string form (cadr arg))))
		     
		     ((list)                          ; (length (list 'a 'b 'c)) -> 3
		      (lint-format "perhaps ~A" caller (lists->string form (- (length arg) 1))))
		     
		     ((append)                        ; (length (append x y)) -> (+ (length x) (length y))
		      (if (= (length arg) 3)
			  (lint-format "perhaps ~A" caller (lists->string form `(+ (length ,(cadr arg)) (length ,(caddr arg)))))))
		     
		     ((quote)                         ; (length '(1 2 3)) -> 3
		      (if (list? (cadr arg))
			  (lint-format "perhaps ~A" caller (lists->string form (length (cadr arg))))))))

		 ;; not pair cadr
		 (if (code-constant? (cadr form))     ; (length 0) -> #f
		     (lint-format "perhaps ~A -> ~A" caller 
				  (truncated-list->string form)
				  (length ((if (and (pair? (cadr form))
						    (eq? (caadr form) 'quote))
					      cadadr cadr)
					   form)))))))
	 (hash-table-set! h 'length sp-length))

	;; ---------------- zero? positive? negative? ----------------
	(let ()
	  (define (sp-zero? caller head form env)
	    (when (pair? (cdr form))
	      (let ((arg (cadr form)))
		(when (pair? arg)
		  
		  (if (and (eq? head 'negative?)       ; (negative? (string-length s))")
			   (hash-table-ref non-negative-ops (car arg)))
		      (lint-format "~A can't be negative: ~A" caller (caadr form) (truncated-list->string form)))
		  
		  (case (car arg)
		    ((-)
		     (lint-format "perhaps ~A" caller  ; (zero? (- x)) -> (zero? x)
				  (lists->string form
						 (let ((op '((zero? = zero?) (positive? > negative?) (negative? < positive?))))
						   (if (null? (cddr arg))
						       `(,(caddr (assq head op)) ,(cadr arg))
						       (if (null? (cdddr arg))
							   `(,(cadr (assq head op)) ,(cadr arg) ,(caddr arg))
							   `(,(cadr (assq head op)) ,(cadr arg) (+ ,@(cddr arg)))))))))

		    ((numerator)              ; (negative? (numerator x)) -> (negative? x)
		     (lint-format "perhaps ~A" caller (lists->string form `(,head ,(cadadr form)))))

		    ((denominator)            ; (zero? (denominator x)) -> error
		     (if (eq? head 'zero)
			 (lint-format "denominator can't be zero: ~A" caller form)))

		    ((string-length)          ; (zero? (string-length x)) -> (string=? x "")
		     (if (eq? head 'zero?)
			 (lint-format "perhaps ~A" caller (lists->string form `(string=? ,(cadadr form) "")))))
		    
		    ((vector-length)          ; (zero? (vector-length c)) -> (equal? c #())
		     (if (eq? head 'zero?)
			 (lint-format "perhaps ~A" caller (lists->string form `(equal? ,(cadadr form) #())))))
		    
		    ((length)                 ;  (zero? (length routes)) -> (null? routes)
		     (if (eq? head 'zero?)
			 (lint-format "perhaps (assuming ~A is list) use null? instead of length: ~A" caller (cadr arg)
				      (lists->string form `(null? ,(cadr arg)))))))))))
	  ;; (zero? (logand...)) is nearly always preceded by not and handled elsewhere
	  (for-each (lambda (f)
		      (hash-table-set! h f sp-zero?))
		    '(zero? positive? negative?)))
	
	;; ---------------- / ----------------
	(let ()
	 (define (sp-/ caller head form env)
	   (when (pair? (cdr form))
	     (if (and (null? (cddr form))
		      (number? (cadr form))
		      (zero? (cadr form)))     ; (/ 0)
		 (lint-format "attempt to invert zero: ~A" caller (truncated-list->string form))
		 (if (and (pair? (cddr form))  ; (/ x y 2 0)
			  (memv 0 (cddr form)))
		     (lint-format "attempt to divide by 0: ~A" caller (truncated-list->string form))))))
	 (hash-table-set! h '/ sp-/))
	
	;; ---------------- copy ----------------
	(let ()
	  (define (sp-copy caller head form env)
	    (cond ((and (pair? (cdr form))          ; (copy (copy x)) could be (copy x)
			(or (number? (cadr form))
			    (boolean? (cadr form))
			    (char? (cadr form))
			    (and (pair? (cadr form))
				 (memq (caadr form) '(copy string-copy))) ; or any maker?
			    (and (pair? (cddr form))
				 (equal? (cadr form) (caddr form)))))
		   (lint-format "~A could be ~A" caller (truncated-list->string form) (cadr form)))
		  
		  ((and (pair? (cdr form))          ; (copy (owlet)) could be (owlet)
			(equal? (cadr form) '(owlet)))
		   (lint-format "~A could be (owlet): owlet is copied internally" caller form))
		  
		  ((= (length form) 5)
		   (check-start-and-end caller 'copy (cdddr form) form env))))
	  (hash-table-set! h 'copy sp-copy))
	
	;; ---------------- string-copy ----------------
	(hash-table-set! 
	 h 'string-copy
	 (lambda (caller head form env)
	   (if (and (pair? (cdr form))  ; (string-copy (string-copy x)) could be (string-copy x)
		    (pair? (cadr form))
		    (memq (caadr form) '(copy string-copy string make-string string-upcase string-downcase
					 string-append list->string symbol->string number->string)))
	       (lint-format "~A could be ~A" caller (truncated-list->string form) (cadr form)))))
	
	;; ---------------- string-down|upcase ----------------
	(let ()
	  (define (sp-string-upcase caller head form env)
	    (if (and (pair? (cdr form))
		     (string? (cadr form))) ; (string-downcase "SPEAK") -> "speak"
	       (lint-format "perhaps ~A" caller (lists->string form 
							       ((if (eq? head 'string-upcase) string-upcase string-downcase)
								(cadr form))))))
	  (hash-table-set! h 'string-upcase sp-string-upcase)
	  (hash-table-set! h 'string-downcase sp-string-upcase))
	
	;; ---------------- string ----------------
	(let ()
	 (define (sp-string caller head form env)
	   (if (every? (lambda (x) 
			 (and (char? x)
			      (char<=? #\space x #\~))) ; #\0xx chars here look dumb
		       (cdr form))
	       (lint-format "~A could be ~S" caller (truncated-list->string form) (apply string (cdr form)))
	       (if (and (pair? (cdr form))              ; (string (string-ref x 0)) -> (substring x 0 1)
			(pair? (cadr form)))
		   (if (and (eq? (caadr form) 'string-ref)
			    (null? (cddr form)))
		       (let ((arg (cdadr form)))
			 (if (integer? (cadr arg))     ; (string (string-ref x 0)) -> (substring x 0 1)
			     (lint-format "perhaps ~A" caller 
					  (lists->string form 
							 `(substring ,(car arg) ,(cadr arg) ,(+ 1 (cadr arg)))))))
		       (if (and (not (null? (cddr form)))
				(memq (caadr form) '(char-upcase char-downcase))
				(every? (lambda (p)
					  (eq? (caadr form) (car p)))
					(cddr form)))
			   ;; (string (char-downcase (string-ref x 1)) (char-downcase (string-ref x 2))) ->
			   ;;    (string-downcase (string (string-ref x 1) (string-ref x 2)))
			   (lint-format "perhaps ~A" caller
					(lists->string form `(,(if (eq? (caadr form) 'char-upcase) 'string-upcase 'string-downcase)
							      (string ,@(map cadr (cdr form)))))))))))
	 ;; repeated args as in vector/list (sp-list below) got no hits
	(hash-table-set! h 'string sp-string))
	
	;; ---------------- string? ----------------
	(let ()
	 (define (sp-string? caller head form env)
	   (if (and (pair? (cdr form))           
		    (pair? (cadr form))
		    (memq (caadr form) '(format number->string)))
	       (if (eq? (caadr form) 'format)                      ; (string? (number->string x)) -> #t
		   (lint-format "format returns either #f or a string, so ~A" caller (lists->string form (cadr form)))
		   (lint-format "number->string always returns a string, so ~A" caller (lists->string form #t)))
	       (check-boolean-affinity caller form env)))
	 (hash-table-set! h 'string? sp-string?))
	
	;; ---------------- number? ----------------
	(let ()
	 (define (sp-number? caller head form env)
	   (if (and (pair? (cdr form))
		    (pair? (cadr form))
		    (eq? (caadr form) 'string->number))  ; (number? (string->number x)) -> (string->number x)
	       (lint-format "string->number returns either #f or a number, so ~A" caller (lists->string form (cadr form)))
	       (check-boolean-affinity caller form env)))
	 (hash-table-set! h 'number? sp-number?))
	
	;; ---------------- symbol? etc ----------------
	(let ()
	  (define (sp-symbol? caller head form env)
	    (check-boolean-affinity caller form env))
	  (for-each (lambda (f)
		      (hash-table-set! h f sp-symbol?))
		    '(symbol? rational? real? complex? float? keyword? gensym? byte-vector? proper-list?
			      char? boolean? float-vector? int-vector? vector? let? hash-table? input-port? c-object?
			      output-port? iterator? continuation? dilambda? procedure? macro? random-state? eof-object? c-pointer?)))

	;; ---------------- pair? list? ----------------	
	(let ()
	  (define (sp-pair? caller head form env)
	    (check-boolean-affinity caller form env)
	    (if (and (pair? (cdr form))     ; (pair? (member x y)) -> (member x y)
		     (pair? (cadr form))
		     (memq (caadr form) '(memq memv member assq assv assoc procedure-signature)))
		(lint-format "~A returns either #f or a pair, so ~A" caller (caadr form)
			     (lists->string form (cadr form))))) 
	  (for-each (lambda (f)
		      (hash-table-set! h f sp-pair?))
		    '(pair? list?)))

	;; ---------------- integer? ----------------	
	(let ()
	 (define (sp-integer? caller head form env)
	   (check-boolean-affinity caller form env)
	   (if (and (pair? (cdr form))    ; (integer? (char-position x y)) -> (char-position x y)
		    (pair? (cadr form))
		    (memq (caadr form) '(char-position string-position)))
	       (lint-format "~A returns either #f or an integer, so ~A" caller (caadr form)
			    (lists->string form (cadr form)))))
	 (hash-table-set! h 'integer? sp-integer?))
	
	;; ---------------- null? ----------------
	(let ()
	 (define (sp-null? caller head form env)
	   (check-boolean-affinity caller form env)
	   (if (and (pair? (cdr form))  ; (null? (string->list x)) -> (zero? (length x))
		    (pair? (cadr form))
		    (memq (caadr form) '(vector->list string->list let->list)))
	       (lint-format "perhaps ~A" caller
			    (lists->string form `(zero? (length ,(cadadr form)))))))
	 (hash-table-set! h 'null? sp-null?))
	
	;; ---------------- odd? even? ----------------
	(let ()
	 (define (sp-odd? caller head form env)
	   (if (and (pair? (cdr form))   ; (odd? (- x 1)) -> (even? x)
		    (pair? (cadr form))
		    (memq (caadr form) '(+ -))
		    (= (length (cadr form)) 3))
	       (let* ((arg1 (cadadr form))
		      (arg2 (caddr (cadr form)))
		      (int-arg (or (and (integer? arg1) arg1)
				   (and (integer? arg2) arg2))))
		 (if int-arg
		     (lint-format "perhaps ~A" caller 
				  (lists->string form 
						 (if (and (integer? arg1)
							  (integer? arg2))
						     (eval form)
						     `(,(if (eq? (eq? head 'even?) (even? int-arg)) 'even? 'odd?)
						       ,(if (integer? arg1) arg2 arg1)))))))))
	 (hash-table-set! h 'odd? sp-odd?)
	 (hash-table-set! h 'even? sp-odd?))
	
	;; ---------------- string-ref ----------------
	(hash-table-set! 
	 h 'string-ref 
	 (lambda (caller head form env)
	   (when (and (= (length form) 3)
		      (pair? (cadr form)))
	     (let ((target (cadr form)))
	       (case (car target)
		 ((substring)                      ;  (string-ref (substring x 1) 2) -> (string-ref x (+ 2 1))
		  (if (= (length target) 3)
		      (lint-format "perhaps ~A" caller (lists->string form `(string-ref ,(cadr target) (+ ,(caddr form) ,(caddr target)))))))
		 
		 ((symbol->string)                 ;  (string-ref (symbol->string 'abs) 1) -> #\b
		  (if (and (integer? (caddr form))
			   (pair? (cadr target))
			   (eq? (caadr target) 'quote)
			   (symbol? (cadadr target)))
		      (lint-format "perhaps ~A" caller (lists->string form (string-ref (symbol->string (cadadr target)) (caddr form))))))
		 
		 ((make-string)                    ;  (string-ref (make-string 3 #\a) 1) -> #\a
		  (if (and (integer? (cadr target))
			   (integer? (caddr form))
			   (> (cadr target) (caddr form)))
		      (lint-format "perhaps ~A" caller (lists->string form (if (= (length target) 3) (caddr target) #\space))))))))))
	
	;; ---------------- vector-ref etc ----------------
	(let ()
	  (define (sp-vector-ref caller head form env)
	    (unless (= line-number last-checker-line-number)
	      (when (= (length form) 3)
		(let ((seq (cadr form)))
		  (when (pair? seq)
		    (if (and (memq (car seq) '(vector-ref int-vector-ref float-vector-ref list-ref hash-table-ref let-ref))
			     (= (length seq) 3))  ; (vector-ref (vector-ref x i) j) -> (x i j)
			(let ((seq1 (cadr seq)))  ;   x 
			  (lint-format "perhaps ~A" caller 
				       (lists->string form 
						      (if (and (pair? seq1)   ; (vector-ref (vector-ref (vector-ref x i) j) k) -> (x i j k)
							       (memq (car seq1) '(vector-ref int-vector-ref float-vector-ref list-ref hash-table-ref let-ref))
							       (= (length seq1) 3))
							  `(,(cadr seq1) ,(caddr seq1) ,(caddr seq) ,(caddr form))
							  `(,seq1 ,(caddr seq) ,(caddr form))))))
			(if (memq (car seq) '(make-vector make-list vector list
							  make-float-vector make-int-vector float-vector int-vector
							  make-hash-table hash-table hash-table*
							  inlet))
			    (lint-format "this doesn't make much sense: ~A" caller form)))
		    (when (eq? head 'list-ref)
		      (if (eq? (car seq) 'quote)
			  (if (proper-list? (cadr seq))  ; (list-ref '(#t #f) (random 2)) -> (vector-ref #(#t #f) (random 2))
			      (lint-format "perhaps use a vector: ~A" caller
					   (lists->string form `(vector-ref ,(apply vector (cadr seq)) ,(caddr form)))))
			  (let ((index (caddr form)))    ; (list-ref (cdddr f) 2) -> (list-ref f 5)
			    (if (and (memq (car seq) '(cdr cddr cdddr))
				     (or (integer? index)
					 (and (pair? index)
					      (eq? (car index) '-)
					      (integer? (caddr index)))))
				(let ((offset (cdr (assq (car seq) '((cdr . 1) (cddr . 2) (cdddr . 3))))))
				  (lint-format "perhaps ~A" caller
					       (lists->string form
							      `(list-ref ,(cadr seq)
									 ,(if (integer? index)
									      (+ index offset)
									      (let ((noff (- (caddr index) offset)))
										(if (zero? noff)
										    (cadr index)
										    `(- ,(cadr index) ,noff)))))))))))))))
	      (set! last-checker-line-number line-number)))
	  (for-each (lambda (f)
		      (hash-table-set! h f sp-vector-ref))
		    '(vector-ref list-ref hash-table-ref let-ref int-vector-ref float-vector-ref)))
	
	
	;; ---------------- vector-set! etc ----------------
	(let ()
	  (define (sp-vector-set! caller head form env)
	    (when (= (length form) 4)
	      (let ((target (cadr form))
		    (index (caddr form))
		    (val (cadddr form)))
		
		(cond ((and (pair? val)                   ; (vector-set! x 0 (vector-ref x 0))
			    (= (length val) 3)
			    (eq? target (cadr val))
			    (equal? index (caddr val))
			    (memq (car val) '(vector-ref list-ref hash-table-ref string-ref let-ref float-vector-ref int-vector-ref)))
		       (lint-format "redundant ~A: ~A" caller head (truncated-list->string form)))

		      ((code-constant? target)            ; (vector-set! #(0 1 2) 1 3)??
		       (lint-format "~A is a constant that is discarded; perhaps ~A" caller target (lists->string form val)))

		      ((not (pair? target)))
		      
		      ((and (not (eq? head 'string-set!)) ; (vector-set! (vector-ref x 0) 1 2) -- vector within vector
			    (memq (car target) '(vector-ref list-ref hash-table-ref let-ref float-vector-ref int-vector-ref)))
		       (lint-format "perhaps ~A" caller (lists->string form `(set! (,@(cdr target) ,index) ,val))))
		      
		      ((memq (car target) '(make-vector vector make-string string make-list list append cons 
					    vector-append inlet sublet copy vector-copy string-copy list-copy)) ;list-copy is from r7rs
		       (lint-format "~A is simply discarded; perhaps ~A" caller
				    (truncated-list->string target)   ; (vector-set! (make-vector 3) 1 1) -- does this ever happen?
				    (lists->string form val)))
		      
		      ((and (eq? head 'list-set!)
			    (memq (car target) '(cdr cddr cdddr cddddr))
			    (integer? (caddr form)))                  ; (list-set! (cdr x) 0 y) -> (list-set! x 1 y)
		       (lint-format "perhaps ~A" caller 
				    (lists->string form 
						   `(list-set! ,(cadr target) ,(+ (caddr form) (cdr-count (car target))) ,(cadddr form)))))))))
	  (for-each (lambda (f)
		      (hash-table-set! h f sp-vector-set!))
		    '(vector-set! list-set! hash-table-set! float-vector-set! int-vector-set! string-set! let-set!)))
	
	;; ---------------- object->string ----------------
	(hash-table-set! 
	 h 'object->string 
	 (lambda (caller head form env)
	   (when (pair? (cdr form))
	     (if (and (pair? (cadr form)) ; (object->string (object->string x)) could be (object->string x)
		      (eq? (caadr form) 'object->string))
		 (lint-format "~A could be ~A" caller (truncated-list->string form) (cadr form))
		 (if (pair? (cddr form))
		     (let ((arg2 (caddr form)))
		       (if (and (code-constant? arg2)                   ; (object->string x :else)
				(not (memq arg2 '(#f #t :readable))))   ; #f and #t are display|write choice, :readable = ~W
			   (lint-format "bad second argument: ~A" caller arg2))))))))
	
	;; ---------------- display ----------------
	(hash-table-set! 
	 h 'display 
	 (lambda (caller head form env)
	   (if (pair? (cdr form))
	       (let ((arg (cadr form))
		     (port (if (pair? (cddr form))
			       (caddr form)
			       ())))
		 (cond ((and (string? arg)
			     (or (string-position "ERROR" arg)
				 (string-position "WARNING" arg)))
			(lint-format  "There's no need to shout: ~A" caller (truncated-list->string form)))
		       
		       ((not (and (pair? arg)
				  (pair? (cdr arg)))))

		       ((and (eq? (car arg) 'format)    ; (display (format #f str x)) -> (format () str x)
			     (not (cadr arg)))
			(lint-format "perhaps ~A" caller (lists->string form `(format ,port ,@(cddr arg)))))
		       
		       ((and (eq? (car arg) 'apply)     ; (display (apply format #f str x) p) -> (apply format p str x)
			     (eq? (cadr arg) 'format)
			     (pair? (cddr arg))
			     (not (caddr arg)))
			(lint-format "perhaps ~A" caller (lists->string form `(apply format ,port ,@(cdddr arg))))))))))
	
	;; ---------------- make-vector etc ----------------
	(let ()
	  (define (sp-make-vector caller head form env)
	    ;; type of initial value (for make-float|int-vector) is checked elsewhere
	    (if (and (= (length form) 4)
		     (eq? head 'make-vector))   ;  (make-vector 3 0 #t)
		(lint-format "make-vector no longer has a fourth argument: ~A~%" caller form))

	    (if (>= (length form) 3)
		(case (caddr form)
		  ((#<unspecified>) 
		   (if (eq? head 'make-vector)  ;  (make-vector 3 #<unspecified>)
		       (lint-format "#<unspecified> is the default initial value in ~A" caller form)))
		  ((0)
		   (if (not (eq? head 'make-vector))
		       (lint-format "0 is the default initial value in ~A" caller form)))
		  ((0.0)
		   (if (eq? head 'make-float-vector)
		       (lint-format "0.0 is the default initial value in ~A" caller form)))))
		    
	    (when (and (pair? (cdr form))
		       (integer? (cadr form))
		       (zero? (cadr form)))
	      (if (pair? (cddr form))           ; (make-vector 0 0.0)
		  (lint-format "initial value is pointless here: ~A" caller form))
	      (lint-format "perhaps ~A" caller (lists->string form #()))))

	  (for-each (lambda (f)
		      (hash-table-set! h f sp-make-vector))
		    '(make-vector make-int-vector make-float-vector)))
	
	;; ---------------- make-string make-byte-vector ----------------
	(let ()
	  (define (sp-make-string caller head form env)
	    (when (and (pair? (cdr form))
		       (integer? (cadr form))
		       (zero? (cadr form)))
	      (if (pair? (cddr form))           ; (make-byte-vector 0 0)
		  (lint-format "initial value is pointless here: ~A" caller form))
	      (lint-format "perhaps ~A" caller (lists->string form "")))) ; #u8() but (equal? #u8() "") -> #t so lint combines these clauses!
	  (for-each (lambda (f)
		      (hash-table-set! h f sp-make-string))
		    '(make-string make-byte-vector)))
	
	;; ---------------- make-list ----------------
	(let ()
	 (define (sp-make-list caller head form env)
	   (when (and (pair? (cdr form))
		      (integer? (cadr form))
		      (zero? (cadr form)))
	     (if (pair? (cddr form))           ; (make-list 0 #f)
		 (lint-format "initial value is pointless here: ~A" caller form))
	     (lint-format "perhaps ~A" caller (lists->string form ()))))
	 (hash-table-set! h 'make-list sp-make-list))
	
	;; ---------------- reverse string->list etc ----------------
	(let ()
	  (define (sp-reverse caller head form env)
	    ;; not string->number -- no point in copying a number and it's caught below
	    (when (pair? (cdr form))

	      (if (and (code-constant? (cadr form))
		       (not (memq head '(list->string list->vector string->list))))
		  (let ((seq (checked-eval form)))
		    (if (not (eq? seq :checked-eval-error))   ;  (symbol->string 'abs) -> "abs"
			(lint-format "perhaps ~A -> ~A~A" caller
				     (truncated-list->string form)
				     (if (pair? seq) "'" "")
				     (if (symbol? seq)
					 (object->string seq :readable)
					 (object->string seq))))))
	      
	      (let ((inverses '((reverse . reverse) 
				(reverse! . reverse!) 
				;; reverse and reverse! are not completely interchangable:
				;;   (reverse (cons 1 2)): (2 . 1)
				;;   (reverse! (cons 1 2)): error: reverse! argument, (1 . 2), is a pair but should be a proper list
				(list->vector . vector->list)
				(vector->list . list->vector)
				(symbol->string . string->symbol)
				(string->symbol . symbol->string)
				(list->string . string->list)
				(string->list . list->string)
				(number->string . string->number))))
		
		(when (and (pair? (cadr form))
			   (pair? (cdadr form)))
		  (let ((inv-op (assq head inverses))
			(arg (cadr form))
			(arg-of-arg (cadadr form))
			(func-of-arg (caadr form)))
		    (if (pair? inv-op) (set! inv-op (cdr inv-op)))
		    
		    (cond ((eq? func-of-arg inv-op)               ; (vector->list (list->vector x)) -> x
			   (if (eq? head 'string->symbol)
			       (lint-format "perhaps ~A" caller (lists->string form arg-of-arg))
			       (lint-format "~A could be (copy ~S)" caller form arg-of-arg)))
			  
			  ((and (eq? head 'list->string)          ; (list->string (vector->list x)) -> (copy x (make-string (length x)))
				(eq? func-of-arg 'vector->list))
			   (lint-format "perhaps ~A" caller (lists->string form `(copy ,arg-of-arg (make-string (length ,arg-of-arg))))))
			  
			  ((and (eq? head 'list->string)          ; (list->string (make-list x y)) -> (make-string x y)
				(eq? func-of-arg 'make-list))
			   (lint-format "perhaps ~A" caller (lists->string form `(make-string ,@(cdr arg)))))
			  
			  ((and (eq? head 'string->list)          ; (string->list (string x y)) -> (list x y)
				(eq? func-of-arg 'string))
			   (lint-format "perhaps ~A" caller (lists->string form `(list ,@(cdr arg)))))
			  
			  ((and (eq? head 'list->vector)          ; (list->vector (make-list ...)) -> (make-vector ...)
				(eq? func-of-arg 'make-list))
			   (lint-format "perhaps ~A" caller (lists->string form `(make-vector ,@(cdr arg)))))
			  
			  ((and (eq? head 'list->vector)          ; (list->vector (string->list x)) -> (copy x (make-vector (length x)))
				(eq? func-of-arg 'string->list))
			   (lint-format "perhaps ~A" caller (lists->string form `(copy ,arg-of-arg (make-vector (length ,arg-of-arg))))))

			  ((and (eq? head 'list->vector)          ; (list->vector (append (vector->list v1) ...)) -> (append v1 ...)
				(eq? func-of-arg 'append)
				(every? (lambda (a)
					  (and (pair? a)
					       (eq? (car a) 'vector->list)))
					(cdadr form)))
			   (lint-format "perhaps ~A" caller
					(lists->string form `(append ,@(map cadr (cdadr form))))))
			  
			  ((and (eq? head 'vector->list)          ; (vector->list (make-vector ...)) -> (make-list ...)
				(eq? func-of-arg 'make-vector))
			   (lint-format "perhaps ~A" caller (lists->string form `(make-list ,@(cdr arg)))))
			  
			  ((and (eq? head 'vector->list)          ; (vector->list (vector ...)) -> (list ...)
				(eq? func-of-arg 'vector))
			   (lint-format "perhaps ~A" caller (lists->string form `(list ,@(cdr arg)))))
			  
			  ((and (eq? head 'vector->list)          ; (vector->list (vector-copy ...)) -> (vector->list ...)
				(eq? func-of-arg 'vector-copy))
			   (lint-format "perhaps ~A" caller (lists->string form `(vector->list ,@(cdr arg)))))
			  
			  ((and (memq func-of-arg '(reverse reverse! copy))
				(pair? (cadr arg))                ; (list->string (reverse (string->list x))) -> (reverse x)
				(eq? (caadr arg) inv-op))
			   (lint-format "perhaps ~A" caller (lists->string form `(,(if (eq? func-of-arg 'reverse!) 'reverse func-of-arg) ,(cadadr arg)))))
			  
			  ((and (memq head '(reverse reverse!))   ; (reverse (string->list x)) -> (string->list (reverse x)) -- often redundant
				(memq func-of-arg '(string->list vector->list sort!)))
			   (if (eq? func-of-arg 'sort!)           ; (reverse (sort! x <)) -> (sort x >)
			       (if (and (pair? (cdr arg))
					(pair? (cddr arg)))
				   (cond ((hash-table-ref reversibles (caddr arg)) 
					  => (lambda (op)
					       (lint-format "possibly ~A" caller (lists->string form `(sort! ,(cadr arg) ,op)))))))
			       (if (null? (cddr arg))
				   (lint-format "perhaps less consing: ~A" caller
						(lists->string form `(,func-of-arg (reverse ,arg-of-arg)))))))
			  
			  ((and (pair? (cadr arg))
				(memq func-of-arg '(cdr cddr cdddr cddddr list-tail))
				(case head
				  ((list->string) (eq? (caadr arg) 'string->list))
				  ((list->vector) (eq? (caadr arg) 'vector->list))
				  (else #f)))
			   (let ((len-diff (if (eq? func-of-arg 'list-tail)
					       (caddr arg)
					       (cdr-count func-of-arg))))
			     (lint-format "perhaps ~A" caller     ; (list->string (cdr (string->list x))) -> (substring x 1)
					  (lists->string form (if (eq? head 'list->string)
								  `(substring ,(cadadr arg) ,len-diff)
								  `(copy ,(cadadr arg) (make-vector (- (length ,(cadadr arg)) ,len-diff))))))))
			  
			  ((and (memq head '(list->vector list->string))
				(eq? func-of-arg 'sort!)          ; (list->vector (sort! (vector->list x) y)) -> (sort! x y)
				(pair? (cadr arg))
				(eq? (caadr arg) (if (eq? head 'list->vector) 'vector->list 'string->list)))
			   (lint-format "perhaps ~A" caller (lists->string form `(sort! ,(cadadr arg) ,(caddr arg)))))
			  
			  ((and (memq head '(list->vector list->string))
				(or (memq func-of-arg '(list cons))
				    (quoted-undotted-pair? arg)))
			   (let ((maker (if (eq? head 'list->vector) 'vector 'string)))
			     (cond ((eq? func-of-arg 'list)
				    (if (var-member maker env)    ; (list->string (list x y z)) -> (string x y z)
					(lint-format "~A could be simplified, but you've shadowed '~A" caller (truncated-list->string form) maker)
					(lint-format "perhaps ~A" caller (lists->string form `(,maker ,@(cdr arg))))))
				   ((eq? func-of-arg 'cons)
				    (if (any-null? (caddr arg))
					(if (var-member maker env) ; (list->string (cons x ())) -> (string x)
					    (lint-format "~A could be simplified, but you've shadowed '~A" caller (truncated-list->string form) maker)
					    (lint-format "perhaps ~A" caller (lists->string form `(,maker ,(cadr arg)))))))
				   ((or (null? (cddr form))        ; (list->string '(#\a #\b #\c)) -> "abc"
					(and (integer? (caddr form))
					     (or (null? (cdddr form))
						 (integer? (cadddr form)))))
				    (lint-format "perhaps ~A" caller 
						 (lists->string form (apply (if (eq? head 'list->vector) vector string) (cadr arg))))))))
			  
			  ((and (memq head '(list->string list->vector))    ; (list->string (reverse x)) -> (reverse (apply string x))
				(memq func-of-arg '(reverse reverse!)))
			   (lint-format "perhaps ~A" caller (lists->string form `(reverse (,head ,arg-of-arg)))))
			  
			  ((and (memq head '(string->list vector->list))
				(= (length form) 4))
			   (check-start-and-end caller head (cddr form) form env))
			  
			  ((and (eq? head 'string->symbol)        ; (string->symbol (string-append...)) -> (symbol ...)
				(or (memq func-of-arg '(string-append append))
				    (and (eq? func-of-arg 'apply)
					 (memq arg-of-arg '(string-append append)))))
			   (lint-format "perhaps ~A" caller
					(lists->string form 
						       (if (eq? func-of-arg 'apply)
							   `(apply symbol ,@(cddr arg))
							   `(symbol ,@(cdr arg))))))

			  ((and (eq? head 'string->symbol)        ; (string->symbol (if (not (null? x)) x "abc")) -> (if (not (null? x)) (string->symbol x) 'abc)
				(eq? func-of-arg 'if)
				(or (string? (caddr arg))
				    (string? (cadddr arg)))
				(not (or (equal? (caddr arg) "")  ; this is actually an error -- should we complain?
					 (equal? (cadddr arg) ""))))
			   (lint-format "perhaps ~A" caller
					(lists->string form
						       (if (string? (caddr arg))
							   (if (string? (cadddr arg))
							       `(if ,(cadr arg) ',(string->symbol (caddr arg)) ',(string->symbol (cadddr arg)))
							       `(if ,(cadr arg) ',(string->symbol (caddr arg)) (string->symbol ,(cadddr arg))))
							   `(if ,(cadr arg) (string->symbol ,(caddr arg)) ',(string->symbol (cadddr arg)))))))
			  
			  ((case head                             ; (reverse (reverse! x)) could be (copy x)
			     ((reverse) (eq? func-of-arg 'reverse!))
			     ((reverse!) (eq? func-of-arg 'reverse))
			     (else #f))
			   (lint-format "~A could be (copy ~S)" caller form arg-of-arg))
			  
			  ((and (pair? arg-of-arg)                ; (op (reverse (inv-op x))) -> (reverse x)
				(eq? func-of-arg 'reverse)
				(eq? inv-op (car arg-of-arg)))
			   (lint-format "perhaps ~A" caller (lists->string form `(reverse ,(cadr arg-of-arg)))))))))
		  
	      (unless (pair? (cadr form))                         ; (string->list "123") -> (#\1 #\2 #\3)
		(let ((arg (cadr form)))
		  (if (and (eq? head 'string->list)
			   (string? arg)
			   (or (null? (cddr form))
			       (and (integer? (caddr form))
				    (or (null? (cdddr form))
					(integer? (cadddr form))))))
		      (lint-format "perhaps ~A -> ~A" caller (truncated-list->string form) (apply string->list (cdr form))))))
	      
	      (when (pair? (cddr form))                           ; (string->list x y y) is ()
		(when (and (memq head '(vector->list string->list))
			   (pair? (cdddr form))
			   (equal? (caddr form) (cadddr form)))
		  (lint-format "leaving aside errors, ~A is ()" caller (truncated-list->string form)))
		
		(when (and (eq? head 'number->string)             ; (number->string saturation 10)
			   (eqv? (caddr form) 10))
		  (lint-format "10 is the default radix for number->string: ~A" caller (truncated-list->string form))))
	      
	      (when (memq head '(reverse reverse!))
		(if (and (eq? head 'reverse!)
			 (symbol? (cadr form)))
		    (let ((v (var-member (cadr form) env)))
		      (if (and (var? v)
			       (eq? (var-definer v) 'parameter))
			  (lint-format "if ~A (a function argument) is a pair, ~A is ill-advised" caller
				       (cadr form) 
				       (truncated-list->string form))))
		    (when (pair? (cadr form))
		      (let ((arg (cadr form)))
			(when (and (pair? (cdr arg))
				   (pair? (cadr arg)))
			  (if (and (memq (car arg) '(cdr list-tail)) ; (reverse (cdr (reverse lst))) = all but last of lst -> copy to len-1
				   (memq (caadr arg) '(reverse reverse!))
				   (symbol? (cadadr arg)))
			      (lint-format "perhaps ~A" caller 
					   (lists->string form `(copy ,(cadadr arg) (make-list (- (length ,(cadadr arg)) ,(if (eq? (car arg) 'cdr) 1 (caddr arg))))))))
			  
			  (if (and (eq? (car arg) 'append)           ; (reverse (append (reverse b) res)) = (append (reverse res) b)
				   (eq? (caadr arg) 'reverse)
				   (pair? (cddr arg))
				   (null? (cdddr arg)))
			      (lint-format "perhaps ~A" caller (lists->string form `(append (reverse ,(caddr arg)) ,(cadadr arg))))))
			
			(when (and (= (length arg) 3)
				   (pair? (caddr arg)))
			  (if (and (eq? (car arg) 'map)              ; (reverse (map abs (sort! x <))) -> (map abs (sort! x >))
				   (eq? (caaddr arg) 'sort!))
			      (cond ((hash-table-ref reversibles (caddr (caddr arg)))
				     => (lambda (op)
					  (lint-format "possibly ~A" caller (lists->string form `(,(car arg) ,(cadr arg) 
												  (sort! ,(cadr (caddr arg)) ,op))))))))
			  ;; (reverse (apply vector (sort! x <))) doesn't happen (nor does this map case, but it's too pretty to leave out)
			  
			  (if (and (eq? (car arg) 'cons)             ; (reverse (cons x (reverse lst))) -- adds x to end -- (append lst (list x))
				   (memq (car (caddr arg)) '(reverse reverse!)))
			      (lint-format "perhaps ~A" caller (lists->string form `(append ,(cadr (caddr arg)) (list ,(cadr arg)))))))))))))
	  
	  (for-each (lambda (f)
		      (hash-table-set! h f sp-reverse))
		    '(reverse reverse! list->vector vector->list list->string string->list symbol->string string->symbol number->string)))
	
	;; ---------------- char->integer string->number etc ----------------
	(let ()
	  (define (sp-char->integer caller head form env)
	    (let ((inverses '((char->integer . integer->char)
			      (integer->char . char->integer)
			      (symbol->keyword . keyword->symbol)
			      (keyword->symbol . symbol->keyword)
			      (string->number . number->string))))
	      (when (pair? (cdr form))
		(let ((arg (cadr form)))
		    (if (and (pair? arg)
			     (pair? (cdr arg))                   ;  (string->number (number->string x)) could be x
			     (eq? (car arg) (cond ((assq head inverses) => cdr))))
			(lint-format "~A could be ~A" caller (truncated-list->string form) (cadr arg))
			(case head
			  ((integer->char)
			   (if (let walk ((tree (cdr form)))
				 (if (pair? tree)
				     (and (walk (car tree))
					  (walk (cdr tree)))
				     (or (code-constant? tree)
					 (not (side-effect? tree env)))))
			       (let ((chr (checked-eval form)))  ; (integer->char (+ (char->integer #\space) 215)) -> #\xf7
				 (if (char? chr)
				     (lint-format "perhaps ~A" caller (lists->string form chr))))))

			  ((string->number)
			   (if (and (pair? (cddr form))
				    (integer? (caddr form)) ; type error is checked elsewhere
				    (not (<= 2 (caddr form) 16)))  ;  (string->number "123" 21)
			       (lint-format "string->number radix should be between 2 and 16: ~A" caller form)
			       (if (and (pair? arg)
					(eq? (car arg) 'string)
					(pair? (cdr arg))
					(null? (cddr form))
					(null? (cddr arg))) ; (string->number (string num-char)) -> (- (char->integer num-char) (char->integer #\0))
				   (lint-format "perhaps ~A" caller
						(lists->string form `(- (char->integer ,(cadr arg)) (char->integer #\0)))))))

			  ((symbol->keyword)
			   (if (and (pair? arg)             ;  (symbol->keyword (string->symbol x)) -> (make-keyword x)
				    (eq? (car arg) 'string->symbol))
			       (lint-format "perhaps ~A" caller (lists->string form `(make-keyword ,(cadr arg))))))))))))

	  (for-each (lambda (f)
		      (hash-table-set! h f sp-char->integer))
		    '(char->integer integer->char symbol->keyword keyword->symbol string->number)))
	
	;; ---------------- string-append ----------------
	(let ()
	 (define (sp-string-append caller head form env)
	   (unless (= line-number last-checker-line-number)
	     (let ((args (remove-all "" (splice-if (lambda (x) (eq? x 'string-append)) (cdr form))))
		   (combined #f))
	       (when (or (any? string? args)
			 (member 'string args (lambda (a b) (and (pair? b) (eq? (car b) a)))))
		 (do ((nargs ())               ; look for (string...) (string...) in the arg list and combine
		      (p args (cdr p)))
		     ((null? p)
		      (set! args (reverse nargs)))
		   (cond ((not (pair? (cdr p)))
			  (set! nargs (cons (car p) nargs)))
			 
			 ((and (pair? (car p))
			       (eq? (caar p) 'string)
			       (pair? (cadr p))
			       (eq? (caadr p) 'string))
			  (set! nargs (cons `(string ,@(cdar p) ,@(cdadr p)) nargs))
			  (set! combined #t)
			  (set! p (cdr p)))
			 
			 ((and (string? (car p))
			       (string? (cadr p)))
			  (set! nargs (cons (string-append (car p) (cadr p)) nargs))
			  (set! combined #t)
			  (set! p (cdr p)))
			 
			 (else (set! nargs (cons (car p) nargs))))))
	     
	       (cond ((null? args)                 ; (string-append) -> ""
		      (lint-format "perhaps ~A" caller (lists->string form "")))
		     
		     ((null? (cdr args))           ; (string-append a) -> a
		      (if (not (tree-memq 'values (cdr form)))
			  (lint-format "perhaps ~A~A" caller (lists->string form (car args))
				       (if combined "" ", or use copy")))) ; (string-append x "") appears to be a common substitute for string-copy
		     
		     ((every? string? args)        ; (string-append "a" "b") -> "ab"
		      (lint-format "perhaps ~A" caller (lists->string form (apply string-append args))))
		     
		     ((every? (lambda (a)          ; (string-append "a" (string #\b)) -> "ab"
				(or (string? a)
				    (and (pair? a)
					 (eq? (car a) 'string)
					 (char? (cadr a)))))
			      args)
		      (catch #t
			(lambda ()                 ; (string-append (string #\C) "ZLl*()def") -> "CZLl*()def"
			  (let ((val (if (not (any? pair? args))
					 (apply string-append args)
					 (eval (cons 'string-append args)))))
			    (lint-format "perhaps ~A -> ~S" caller (truncated-list->string form) val)))
			(lambda args #f)))

		     ((every? (lambda (c)          ; (string-append (make-string 3 #\a) (make-string 2 #\b)) -> (format #f "~NC~NC" 3 #\a 2 #\b)
				(and (pair? c)
				     (eq? (car c) 'make-string)
				     (pair? (cdr c))
				     (pair? (cddr c))))
			      (cdr form))
		      (lint-format "perhaps ~A" caller
				   (lists->string form
						  `(format #f ,(apply string-append (make-list (abs (length (cdr form))) "~NC"))
							   ,@(map (lambda (c) (values (cadr c) (caddr c))) (cdr form))))))

		     ((not (equal? args (cdr form)))  ; (string-append x (string-append y z)) -> (string-append x y z)
		      (lint-format "perhaps ~A" caller (lists->string form `(string-append ,@args)))))
	       (set! last-checker-line-number line-number))))
	 (hash-table-set! h 'string-append sp-string-append))
	
	;; ---------------- vector-append ----------------
	(let ()
	 (define (sp-vector-append caller head form env)
	   (unless (= line-number last-checker-line-number)
	     (let ((args (remove-all #() (splice-if (lambda (x) (eq? x 'vector-append)) (cdr form)))))
	       (cond ((null? args)                    ;  (vector-append) -> #()
		      (lint-format "perhaps ~A" caller (lists->string form #())))
		     
		     ((null? (cdr args))              ; (vector-append x) -> (copy x)
		      (lint-format "perhaps ~A" caller (lists->string form `(copy ,(car args)))))
		     
		     ((every? vector? args)           ;  (vector-append #(1 2) (vector-append #(3))) -> #(1 2 3)
		      (lint-format "perhaps ~A" caller (lists->string form (apply vector-append args))))
		     
		     ((not (equal? args (cdr form)))  ; (vector-append x (vector-append y z)) -> (vector-append x y z)
		      (lint-format "perhaps ~A" caller (lists->string form `(vector-append ,@args)))))
	       (set! last-checker-line-number line-number))))
	 (hash-table-set! h 'vector-append sp-vector-append))
	
	;; ---------------- cons ----------------
	(let ()
	 (define (sp-cons caller head form env)
	   (cond ((or (not (= (length form) 3))
		      (= last-cons-line-number line-number))
		  #f)
		 
		 ((and (pair? (caddr form))
		       (or (eq? (caaddr form) 'list)         ; (cons x (list ...)) -> (list x ...)
			   (and (eq? (caaddr form) #_{list})
				(not (tree-member #_{apply_values} (cdaddr form))))))
		  (lint-format "perhaps ~A" caller (lists->string form `(list ,(cadr form) ,@(un_{list} (cdaddr form))))))
		 
		 ((any-null? (caddr form))                   ; (cons x '()) -> (list x)
		  (lint-format "perhaps ~A" caller (lists->string form `(list ,(cadr form)))))
		 
		 ((not (pair? (caddr form))))
		 
		 ((and (pair? (cadr form))                   ; (cons (car x) (cdr x)) -> (copy x)
		       (let ((x (assq (caadr form)
				      '((car cdr #t) 
					(caar cdar car) (cadr cddr cdr)
					(caaar cdaar caar) (caadr cdadr cadr) (caddr cdddr cddr) (cadar cddar cdar)
					(cadddr cddddr cdddr) (caaaar cdaaar caaar) (caaadr cdaadr caadr) (caadar cdadar cadar)
					(caaddr cdaddr caddr) (cadaar cddaar cdaar) (cadadr cddadr cdadr) (caddar cdddar cddar)))))
			 (and x 
			      (eq? (cadr x) (caaddr form))
			      (caddr x))))
		  => (lambda (cfunc)
		       (if (and cfunc
				(equal? (cadadr form) (cadr (caddr form)))
				(not (side-effect? (cadadr form) env)))
			   (lint-format "perhaps ~A" caller (lists->string form 
									   (if (symbol? cfunc)
									       `(copy (,cfunc ,(cadadr form)))
									       `(copy ,(cadadr form))))))))
		 
		 ((eq? (caaddr form) 'cons)   ; list handled above
					; (cons a (cons b (cons ...))) -> (list a b ...), input ending in nil of course
		  (let loop ((args (list (cadr form))) (chain (caddr form)))
		    (if (pair? chain)
			(if (eq? (car chain) 'list)
			    (begin
			      (lint-format "perhaps ~A" caller (lists->string form `(list ,@(reverse args) ,@(cdr chain))))
			      (set! last-cons-line-number line-number))
			    (if (and (eq? (car chain) 'cons)
				     (pair? (cdr chain))
				     (pair? (cddr chain)))
				(if (any-null? (caddr chain))
				    (begin
				      (lint-format "perhaps ~A" caller (lists->string form `(list ,@(reverse args) ,(cadr chain))))
				      (set! last-cons-line-number line-number))
				    (if (and (pair? (caddr chain))
					     (memq (caaddr chain) '(cons list)))
					(loop (cons (cadr chain) args) (caddr chain)))))))))))
	 (hash-table-set! h 'cons sp-cons))
	
	;; ---------------- append ----------------
	(let ()
	 (define (sp-append caller head form env)
	   (unless (= line-number last-checker-line-number)
	     (set! last-checker-line-number line-number)
	     (letrec ((splice-append (lambda (lst)
				       (cond ((null? lst)
					      ())
					     ((not (pair? lst))
					      lst)
					     ((and (pair? (car lst))
						   (eq? (caar lst) 'append))
					      (if (null? (cdar lst))
						  (if (null? (cdr lst)) ; (append) at end -> () to keep copy intact?
						      (list ())
						      (splice-append (cdr lst)))
						  (append (splice-append (cdar lst)) (splice-append (cdr lst)))))
					     ((and (pair? (car lst))
						   (eq? (caar lst) 'copy)
						   (pair? (cdr lst))
						   (null? (cddar lst)))
					      (cons (cadar lst) (splice-append (cdr lst))))
					     ((or (null? (cdr lst))
						  (not (or (any-null? (car lst))
							   (and (pair? (car lst))
								(eq? (caar lst) 'list)
								(null? (cdar lst))))))
					      (cons (car lst) (splice-append (cdr lst))))
					     (else (splice-append (cdr lst)))))))
	       
	       (let ((new-args (splice-append (cdr form))))     ; (append '(1) (append '(2) '(3))) -> (append '(1) '(2) '(3))
		 (let ((len1 (length new-args))
		       (suggestion made-suggestion))
		   (if (and (> len1 2)
			    (null? (list-ref new-args (- len1 1)))
			    (pair? (list-ref new-args (- len1 2)))
			    (memq (car (list-ref new-args (- len1 2))) '(list cons append map string->list vector->list make-list)))
		       (begin
			 (set-cdr! (list-tail new-args (- len1 2)) ())
			 (set! len1 (- len1 1))))
		   
		   (define (append->list . items)
		     (let ((lst (list 'list)))
		       (for-each 
			(lambda (item)
			  (set! lst (append lst (if (eq? (car item) 'list)
						    (cdr item)
						    (distribute-quote (cadr item))))))
			items)
		       lst))
		   
		   (if (positive? len1)
		       (let ((last (list-ref new-args (- len1 1))))
			 ;; (define (f) (append '(1) '(2))) (define a (f)) (set! (a 1) 32) (f) -> '(1 32)
			 (if (and (pair? last)
				  (eq? (car last) 'quote)
				  (pair? (cdr last))
				  (pair? (cadr last)))
			     (lint-format "append does not copy its last argument, so ~A is dangerous" caller form))))
		   
		   (case len1
		     ((0)					        ; (append) -> ()
		      (lint-format "perhaps ~A" caller (lists->string form ())))
		     ((1)                                               ; (append x) -> x
		      (lint-format "perhaps ~A" caller (lists->string form (car new-args))))
		     ((2)                                               ; (append (list x) ()) -> (list x)
		      (let ((arg2 (cadr new-args))
			    (arg1 (car new-args)))
			(cond ((or (any-null? arg2)           
				   (equal? arg2 '(list)))               ; (append x ()) -> (copy x)
			       (lint-format "perhaps clearer: ~A" caller (lists->string form `(copy ,arg1))))
			      
			      ((null? arg1)                             ; (append () x) -> x
			       (lint-format "perhaps ~A" caller (lists->string form arg2)))
			      
			      ((not (pair? arg1)))
			      
			      ((and (pair? arg2)                        ; (append (list x y) '(z)) -> (list x y z) or extensions thereof
				    (or (eq? (car arg1) 'list)
					(quoted-undotted-pair? arg1))
				    (or (eq? (car arg2) 'list)
					(quoted-undotted-pair? arg2)))
			       (lint-format "perhaps ~A" caller (lists->string form (apply append->list new-args))))
			      
			      ((and (eq? (car arg1) 'list)              ; (append (list x) y) -> (cons x y)
				    (pair? (cdr arg1))
				    (null? (cddr arg1)))
			       (lint-format "perhaps ~A" caller (lists->string form `(cons ,(cadr arg1) ,arg2))))
			      
			      ((and (eq? (car arg1) 'list)              ; (append (list x y) z) -> (cons x (cons y z))
				    (pair? (cdr arg1)) 
				    (pair? (cddr arg1))
				    (null? (cdddr arg1)))
			       (lint-format "perhaps ~A" caller (lists->string form `(cons ,(cadr arg1) (cons ,(caddr arg1) ,arg2)))))

			      ;; not sure about this: reports the un-qq'd form (and never happens)
			      ((and (eq? (car arg1) #_{list})
				    (not (qq-tree? arg1)))
			       (set! last-checker-line-number -1)
			       (sp-append caller 'append `(append ,(un_{list} arg1) ,arg2) env))
			      
			      ((and (eq? (car arg1) 'vector->list)
				    (pair? arg2)
				    (eq? (car arg2) 'vector->list))
			       (lint-format "perhaps ~A" caller (lists->string form `(vector->list (append ,(cadr arg1) ,(cadr arg2))))))
			      
			      ((and (eq? (car arg1) 'quote)            ; (append '(x) y) -> (cons 'x y)
				    (pair? (cadr arg1))
				    (null? (cdadr arg1)))
			       (lint-format "perhaps ~A" caller 
					    (lists->string form
							   (if (or (symbol? (caadr arg1))
								   (pair? (caadr arg1)))
							       `(cons ',(caadr arg1) ,arg2)
							       `(cons ,(caadr arg1) ,arg2)))))
			      
			      ((not (equal? (cdr form) new-args))      ; (append () '(1 2) 1) -> (append '(1 2) 1)
			       (lint-format "perhaps ~A" caller (lists->string form `(append ,@new-args)))))))
		     (else
		      (cond ((every? (lambda (item)
				       (and (pair? item)
					    (or (eq? (car item) 'list)
						(quoted-undotted-pair? item))))
				     new-args)                         ; (append '(1) (append '(2) '(3)) '(4)) -> (list 1 2 3 4)
			     (lint-format "perhaps ~A" caller (lists->string form (apply append->list new-args))))
			    
			    ((and (pair? (car new-args))               ; (append (list x) y (list z)) -> (cons x (append y (list z)))?
				  (eq? (caar new-args) 'list)
				  (null? (cddar new-args)))
			     (lint-format "perhaps ~A" caller (lists->string form `(cons ,(cadar new-args) (append ,@(cdr new-args))))))
			    
			    ((let ((n-1 (list-ref new-args (- len1 2))))
			       (and (pair? n-1)
				    (eq? (car n-1) 'list)
				    (pair? (cdr n-1))
				    (null? (cddr n-1))))               ; (append x (list y) z) -> (append x (cons y z))
			     (lint-format "perhaps ~A" caller 
					  (lists->string form 
							 `(append ,@(copy new-args (make-list (- len1 2)))
								  (cons ,(cadr (list-ref new-args (- len1 2))) 
									,(list-ref new-args (- len1 1)))))))

			    ((not (equal? (cdr form) new-args))        ; (append x y (append)) -> (append x y ())
			     (lint-format "perhaps ~A" caller (lists->string form `(append ,@new-args)))))))
		   
		   (if (and (= made-suggestion suggestion)
			    (not (equal? (cdr form) new-args)))
		       (lint-format "perhaps ~A" caller (lists->string form `(append ,@new-args)))))))))
	 (hash-table-set! h 'append sp-append))

	;; ---------------- apply ----------------
	(let ()
	 (define (sp-apply caller head form env)
	   (when (pair? (cdr form))
	     (let ((len (length form))
		   (suggestion made-suggestion))
	       (if (= len 2)                ; (apply f) -> (f)
		   (lint-format "perhaps ~A" caller (lists->string form (list (cadr form))))
		   (if (not (or (<= len 2) ; it might be (apply)...
				(symbol? (cadr form))
				(applicable? (cadr form))))
		       (lint-format "~S is not applicable: ~A" caller (cadr form) (truncated-list->string form))
		       (let ((happy #f)
			     (f (cadr form)))
			 (unless (or (<= len 2)
				     (any-macro? f env)
				     (eq? f 'macroexpand)) ; handled specially (syntactic, not a macro)
			   
			   (when (and (symbol? f)
				      (not (var-member f env)))
			     (let ((func (symbol->value f *e*)))
			       (if (procedure? func)
				   (let ((ary (arity func)))
				     (when (pair? ary)             ; (apply real? 1 3 rest)
				       (if (> (- len 3) (cdr ary)) ; last apply arg might be var=()
					   (lint-format "too many arguments for ~A: ~A" caller f form))
				       (if (and (= len 3)
						(= (car ary) 1)
						(= (cdr ary) 1))   ; (apply car x) -> (car (car x))
					   (lint-format "perhaps ~A" caller (lists->string form `(,f (car ,(caddr form)))))))))))
			   
			   (let ((last-arg (form (- len 1))))
			     (if (and (not (list? last-arg))
				      (code-constant? last-arg))   ; (apply + 1)
				 (lint-format "last argument should be a list: ~A" caller (truncated-list->string form))
				 (if (= len 3)
				     (let ((args (caddr form)))
				       (if (identity? f)                         ; (apply (lambda (x) x) y) -> (car y)
					   (lint-format "perhaps (assuming ~A is a list of one element) ~A" caller args 
							(lists->string form `(car ,args)))
					   (if (simple-lambda? f)                ; (apply (lambda (x) (f x)) y) -> (f (car y))
					       (lint-format "perhaps (assuming ~A is a list of one element) ~A" caller args 
							    (lists->string form (tree-subst (list 'car args) (caadr f) (caddr f))))))
				       
				       (cond ((eq? f 'list)                      ; (apply list x) -> x?
					      (lint-format "perhaps ~A" caller (lists->string form args)))
					     
					     ((any-null? args)                   ; (apply f ()) -> (f)
					      (lint-format "perhaps ~A" caller (lists->string form (list f))))

					     ((or (not (pair? args))
						  (case (car args) 
						    ((list)             ; (apply f (list a b)) -> (f a b)
						     (lint-format "perhaps ~A" caller (lists->string form `(,f ,@(cdr args)))))
						    
						    ((quote)            ; (apply eq? '(a b)) -> (eq? 'a 'b)
						     (and (= suggestion made-suggestion)
							  (lint-format "perhaps ~A" caller (lists->string form `(,f ,@(distribute-quote (cadr args)))))))
						    
						    ((cons)             ; (apply f (cons a b)) -> (apply f a b)
						     (lint-format "perhaps ~A" caller 
								  (lists->string form 
										 (if (and (pair? (caddr args))
											  (eq? (caaddr args) 'cons))
										     `(apply ,f ,(cadr args) ,@(cdaddr args))
										     `(apply ,f ,@(cdr args))))))
						    
						    ((append)           ; (apply f (append (list ...)...)) -> (apply f ... ...)
						     (and (pair? (cadr args))
							  (eq? (caadr args) 'list)
							  (lint-format "perhaps ~A" caller 
								       (lists->string form `(apply ,f ,@(cdadr args)
												   ,(if (null? (cddr args)) ()
													(if (null? (cdddr args)) (caddr args)
													    `(append ,@(cddr args)))))))))
						    
						    ((reverse reverse!)   ; (apply vector (reverse x)) -> (reverse (apply vector x))
						     (and (memq f '(string vector int-vector float-vector))
							  (lint-format "perhaps ~A" caller (lists->string form `(reverse (apply ,f ,(cadr args)))))))

						    ((make-list)          ; (apply string (make-list x y)) -> (make-string x y)
						     (if (memq f '(string vector))
							 (lint-format "perhaps ~A" caller 
								      (lists->string form
										     `(,(if (eq? f 'string) 'make-string 'make-vector)
										       ,@(cdr args))))))
						    
						    ((map)
						     (case f 
						       ((string-append)        ; (apply string-append (map ...))
							(if (eq? (cadr args) 'symbol->string)
							    (lint-format "perhaps ~A" caller ; (apply string-append (map symbol->string ...))
									 (lists->string form `(format #f "~{~A~}" ,(caddr args))))
							    (if (simple-lambda? (cadr args))
								(let ((body (caddr (cadr args))))
								  (if (and (pair? body)
									   (eq? (car body) 'string-append)
									   (= (length body) 3)
									   (or (and (string? (cadr body))
										    (eq? (caddr body) (caadr (cadr args))))
									       (and (string? (caddr body))
										    (eq? (cadr body) (caadr (cadr args))))))
								      (let ((str (string-append "~{" 
												(if (string? (cadr body)) (cadr body) "~A")
												(if (string? (caddr body)) (caddr body) "~A")
												"~}")))
									(lint-format "perhaps ~A" caller
										     (lists->string form `(format #f ,str ,(caddr args))))))))))
						       
						       ((string)          ; (apply string (map char-downcase x)) -> (string-downcase (apply string x))
							(if (memq (cadr args) '(char-upcase char-downcase))
							    (lint-format "perhaps, assuming ~A is a list, ~A" caller (caddr args)
									 (lists->string form `(,(if (eq? (cadr args) 'char-upcase)
												    'string-upcase 'string-downcase)
											       (apply string ,(caddr args)))))))
						       
						       ((append)        ; (apply append (map vector->list args)) -> (vector->list (apply append args))
							(and  (eq? (cadr args) 'vector->list)
							      (lint-format "perhaps ~A" caller (lists->string form `(vector->list (apply append ,@(cddr args)))))))

						       (else #f)))
						    ;; (apply append (map...)) is very common but changing it to
						    ;;     (map (lambda (x) (apply values (f x))) ...) from (apply append (map f ...))
						    ;;     is not an obvious win.  The code is more complicated, and currently apply values 
						    ;;     copies its args (as do apply and append -- how many copies are there here?!
						    
						    ;; need to check for only one apply values
						    ((#_{list})          ; (apply f `(,x ,@z)) -> (apply f x z)
						     (let ((last-arg (list-ref args (- (length args) 1))))
						       (if (and (pair? last-arg)
								(eq? (car last-arg) #_{apply_values})
								(= (tree-count1 #_{apply_values} args 0) 1))
							   (lint-format "perhaps ~A" caller
									(lists->string form
										       `(apply ,f
											       ,@(copy args (make-list (- (length args) 2)) 1)
											       ,(cadr last-arg))))
							   (if (not (tree-member #_{apply_values} (cdr args)))
							       (lint-format "perhaps ~A" caller
									    (lists->string form
											   `(,f ,@(un_{list} (cdr args))))))))))))))
				     (begin ; len > 3
				       (when (and (pair? last-arg)
						  (eq? (car last-arg) 'list)            ; (apply f y z (list a b)) -> (f y z a b)
						  (not (hash-table-ref syntaces f))) ; also not any-macro I presume
					 (lint-format "perhaps ~A" caller 
						      (lists->string form 
								     (append (copy (cdr form) (make-list (- len 2))) 
									     (cdr last-arg)))))
				       
				       ;; can't cleanly go from (apply write o p) to (write o (car p)) since p can be ()
				       
				       (when (and (not happy)
						  (not (memq f '(define define* define-macro define-macro* define-bacro define-bacro* lambda lambda*)))
						  (any-null? last-arg))                 ; (apply f ... ()) -> (f ...)
					 (lint-format "perhaps ~A" caller (lists->string form `(,f ,@(copy (cddr form) (make-list (- len 3)))))))))))))))
	       (if (and (= suggestion made-suggestion)
			(symbol? (cadr form)))
		   (let ((ary (arg-arity (cadr form) env)))
		     (if (and (pair? ary)             ; (apply make-string tcnt initializer) -> (make-string tcnt (car initializer))
			      (= (cdr ary) (- len 2)))
			 (lint-format "perhaps ~A" caller
				      (lists->string form `(,@(copy (cdr form) (make-list (- len 2))) (car ,(list-ref form (- len 1))))))))))))

	 (hash-table-set! h 'apply sp-apply))

	;; ---------------- format ----------------
	(let ()
	  (define (sp-format caller head form env)
	    (if (< (length form) 3)
		(begin
		  (cond ((< (length form) 2)               ; (format)
			 (lint-format "~A has too few arguments: ~A" caller head (truncated-list->string form)))

			((and (pair? (cadr form))          ; (format (format #f str))
			      (eq? (caadr form) 'format))
			 (lint-format "redundant format: ~A" caller (truncated-list->string form)))

			((and (code-constant? (cadr form)) ; (format 1)
			      (not (string? (cadr form))))
			 (lint-format "format with one argument takes a string: ~A" caller (truncated-list->string form)))

			((and (string? (cadr form))        ; (format "str") -> str
			      (eq? head 'format)    ; not snd-display
			      (not (char-position #\~ (cadr form))))
			 (lint-format "perhaps ~A" caller (lists->string form (cadr form)))))
		  env)
		
		(let ((control-string ((if (string? (cadr form)) cadr caddr) form))
		      (args ((if (string? (cadr form)) cddr cdddr) form)))
		  
		  (define count-directives 
		    (let ((format-control-char (let ((chars (make-vector 256 #f)))
						 (for-each
						  (lambda (c)
						    (vector-set! chars (char->integer c) #t))
						  '(#\A #\S #\C #\F #\E #\G #\O #\D #\B #\X #\P #\N #\W #\, #\{ #\} #\* #\@
						    #\a #\s #\c #\f #\e #\g #\o #\d #\b #\x #\p #\n #\w
						    #\0 #\1 #\2 #\3 #\4 #\5 #\6 #\7 #\8 #\9))
						 chars)))
		      (lambda (str caller form)
			(let ((curlys 0)
			      (dirs 0)
			      (pos (char-position #\~ str)))
			  (when pos
			    (do ((len (length str))
				 (tilde-time #t)
				 (i (+ pos 1) (+ i 1)))
				((>= i len)
				 (if tilde-time         ; (format #f "asdf~")
				     (lint-format "~A control string ends in tilde: ~A" caller head (truncated-list->string form))))
			      (let ((c (string-ref str i)))
				(if tilde-time
				    (begin
				      (when (and (= curlys 0)
						 (not (memv c '(#\~ #\T #\t #\& #\% #\^ #\| #\newline #\}))) ; ~* consumes an arg
						 (not (call-with-exit
						       (lambda (return)
							 (do ((k i (+ k 1)))
							     ((= k len) #f)
							   ;; this can be confused by pad chars in ~T
							   (if (not (or (char-numeric? (string-ref str k))
									(char=? (string-ref str k) #\,)))
							       (return (char-ci=? (string-ref str k) #\t))))))))
					;; the possibilities are endless, so I'll stick to the simplest
					(if (not (vector-ref format-control-char (char->integer c))) ; (format #f "~H" 1)
					    (lint-format "unrecognized format directive: ~C in ~S, ~S" caller c str form))
					(set! dirs (+ dirs 1))
					
					;; ~n so try to figure out how many args are needed (this is not complete)
					(when (char-ci=? c #\n)
					  (let ((j (+ i 1)))
					    (if (>= j len)             ; (format p "~A~A" x)
						(lint-format "missing format directive: ~S" caller str)
						(begin
						  ;; if ,n -- add another, if then not T, add another
						  (if (char=? (string-ref str j) #\,)
						      (cond ((>= (+ j 1) len)
							     (lint-format "missing format directive: ~S" caller str))
							    ((char-ci=? (string-ref str (+ j 1)) #\n)
							     (set! dirs (+ dirs 1))
							     (set! j (+ j 2)))
							    ((char-numeric? (string-ref str (+ j 1)))
							     (set! j (+ j 2)))
							    (else (set! j (+ j 1)))))
						  (if (>= j len)
						      (lint-format "missing format directive: ~S" caller str)
						      (if (not (char-ci=? (string-ref str j) #\t))
							  (set! dirs (+ dirs 1)))))))))
				      
				      (set! tilde-time #f)
				      (case c 
					((#\{) (set! curlys (+ curlys 1)))
					((#\}) (set! curlys (- curlys 1)))
					((#\^ #\|)
					 (if (zero? curlys)   ; (format #f "~^")
					     (lint-format "~A has ~~~C outside ~~{~~}?" caller str c))))
				      (if (and (< (+ i 2) len)
					       (member (substring str i (+ i 3)) '("%~&" "^~^" "|~|" "&~&" "\n~\n") string=?))
					  (lint-format "~A in ~A could be ~A" caller  ;  (format #f "~%~&")
						       (substring str (- i 1) (+ i 3))
						       str
						       (substring str (- i 1) (+ i 1)))))
				    (begin
				      (set! pos (char-position #\~ str i))
				      (if pos 
					  (begin
					    (set! tilde-time #t)
					    (set! i pos))
					  (set! i len)))))))
			  
			  (if (not (= curlys 0))   ;  (format #f "~{~A" 1)
			      (lint-format "~A has ~D unmatched ~A~A: ~A"
					   caller head 
					   (abs curlys) 
					   (if (positive? curlys) "{" "}") 
					   (if (> curlys 1) "s" "") 
					   (truncated-list->string form)))
			  dirs))))
		  
		  (when (and (eq? head 'format)
			     (string? (cadr form))) ; (format "s")
		    (lint-format "please include the port argument to format, perhaps ~A" caller `(format () ,@(cdr form))))
		  
		  (if (any? (lambda (arg)
			      (and (string? arg)
				   (or (string-position "ERROR" arg)
				       (string-position "WARNING" arg))))
			    (cdr form))
		      (lint-format "There's no need to shout: ~A" caller (truncated-list->string form)))

		  (if (and (eq? (cadr form) 't)   ; (format t " ")
			   (not (var-member 't env)))
		      (lint-format "'t in ~A should probably be #t" caller (truncated-list->string form)))
		  
		  (if (not (string? control-string))
		      (if (not (proper-list? args))
			  (lint-format "~S looks suspicious" caller form))
		      (let ((ndirs (count-directives control-string caller form))
			    (nargs (if (list? args) (length args) 0)))
			(let ((pos (char-position #\null control-string)))
			  (if (and pos (< pos (length control-string)))  ; (format #f "~a\x00b" x)
			      (lint-format "#\\null in a format control string will confuse both lint and format: ~S in ~A" caller control-string form)))
			(if (not (or (= ndirs nargs)
				     (tree-memq 'values form)))
			    (lint-format "~A has ~A arguments: ~A"       ; (format #f "~nT" 1 2)
					 caller head 
					 (if (> ndirs nargs) "too few" "too many")
					 (truncated-list->string form))
			    (if (and (not (cadr form))                   ; (format #f "123")
				     (zero? ndirs)
				     (not (char-position #\~ control-string)))
				(lint-format "~A could be ~S, (format is a no-op here)" caller (truncated-list->string form) (caddr form))))))
		  
		  (when (pair? args)
		    (for-each
		     (lambda (a)
		       (if (pair? a)
			   (case (car a)
			     ((number->string)
			      (if (null? (cddr a))                      ; (format #f "~A" (number->string x))
				  (lint-format "format arg ~A could be ~A" caller a (cadr a))
				  (if (and (pair? (cddr a))
					   (integer? (caddr a))
					   (memv (caddr a) '(2 8 10 16)))
				      (if (= (caddr a) 10)
					  (lint-format "format arg ~A could be ~A" caller a (cadr a))
					  (lint-format "format arg ~A could use the format directive ~~~A and change the argument to ~A" caller a
						       (case (caddr a) ((2) "B") ((8) "O") (else "X"))
						       (cadr a))))))
			     
			     ((symbol->string)                          ; (format #f "~A" (symbol->string 'x))
			      (lint-format "format arg ~A could be ~A" caller a (cadr a)))
			     
			     ((make-string)                             ; (format #f "~A" (make-string len c))
			      (lint-format "format arg ~A could use the format directive ~~NC and change the argument to ... ~A ~A ..." caller a
					   (cadr a) (if (char? (caddr a)) (format #f "~W" (caddr a)) (caddr a))))
			     
			     ((string-append)                           ; (format #f "~A" (string-append x y))
			      (lint-format "format appends strings, so ~A seems wasteful" caller a)))))
		     args)))))
	  (hash-table-set! h 'format sp-format))
	
	;; ---------------- error ----------------
	(hash-table-set! 
	 h 'error
 	 (lambda (caller head form env)
	   (if (any? (lambda (arg)
		       (and (string? arg)
			    (or (string-position "ERROR" arg)
				(string-position "WARNING" arg))))
		     (cdr form))
	       (lint-format "There's no need to shout: ~A" caller (truncated-list->string form)))))
	
	;; ---------------- sort! ----------------
	(let ()
	 (define (sp-sort caller head form env)
	   (if (= (length form) 3)
	       (let ((func (caddr form)))
		 (if (memq func '(= eq? eqv? equal? string=? char=? string-ci=? char-ci=?))
		     (lint-format "sort! with ~A may hang: ~A" caller func (truncated-list->string form))
		     (if (symbol? func)
			 (let ((sig (procedure-signature (symbol->value func))))
			   (if (and (pair? sig)
				    (not (eq? 'boolean? (car sig)))
				    (not (and (pair? (car sig))
					      (memq 'boolean? (car sig)))))  ;  (sort! x abs)
			       (lint-format "~A is a questionable sort! function" caller func))))))))
	 (hash-table-set! h 'sort! sp-sort))
	
	;; ---------------- substring ----------------
	(let ()
	 (define (sp-substring caller head form env)
	   (if (every? code-constant? (cdr form))
	       (catch #t
		 (lambda ()
		   (let ((val (eval form)))     ; (substring "abracadabra" 2 7) -> "racad"
		     (lint-format "perhaps ~A -> ~S" caller (truncated-list->string form) val)))
		 (lambda (type info)
		   (lint-format "~A -> ~A" caller (truncated-list->string form) (apply format #f info))))
	       
	       (let ((str (cadr form)))

		 (when (string? str)           ; (substring "++++++" 0 2) -> (make-string 2 #\+)
		   (let ((len (length str)))
		     (when (and (> len 0)
				(string=? str (make-string len (string-ref str 0))))
		       (lint-format "perhaps ~A" caller
				    (lists->string form 
						   (let ((chars (if (null? (cddr form))
								    len
								    (if (pair? (cdddr form))
									(if (eqv? (caddr form) 0)
									    (cadddr form)
									    `(- ,(cadddr form) ,(caddr form)))
									`(- ,len ,(caddr form))))))
						     `(make-string ,chars ,(string-ref str 0))))))))
		 (when (pair? (cddr form))
		   (when (null? (cdddr form))
		     (when (and (pair? str)                ; (substring (substring x 1) 2) -> (substring x 3)
				(eq? (car str) 'substring)
				(null? (cdddr str)))
		       (lint-format "perhaps ~A" caller 
				    (lists->string form 
						   (if (and (integer? (caddr form))
							    (integer? (caddr str)))
						       `(substring ,(cadr str) ,(+ (caddr str) (caddr form)))
						       `(substring ,(cadr str) (+ ,(caddr str) ,(caddr form)))))))
		     
		     ;; end indices are complicated -- since this rarely happens, not worth the trouble
		     (if (eqv? (caddr form) 0)   ;  (substring x 0) -> (copy x)
			 (lint-format "perhaps clearer: ~A" caller (lists->string form `(copy ,str)))))
		   
		   (when (pair? (cdddr form))
		     (let ((end (cadddr form)))
		       (if (equal? (caddr form) end)   ; (substring x (+ y 1) (+ y 1)) is ""
			   (lint-format "leaving aside errors, ~A is \"\"" caller form))
		       
		       (when (and (pair? str)
				  (eqv? (caddr form) 0)
				  (eq? (car str) 'string-append)
				  (= (length str) 3))
			 (let ((in-arg2 (caddr str)))
			   (if (and (pair? in-arg2)    ;  (substring (string-append str (make-string len #\space)) 0 len) -> (copy str (make-string len #\space))
				    (eq? (car in-arg2) 'make-string)
				    (equal? (cadddr form) (cadr in-arg2)))
			       (lint-format "perhaps ~A" caller
					    (lists->string form `(copy ,(cadr str) (make-string ,(cadddr form) ,(caddr in-arg2))))))))
		       
		       (if (and (pair? end)           ; (substring x start (length|string-length x)) -> (substring s start)
				(memq (car end) '(string-length length))
				(equal? (cadr end) str))
			   (lint-format "perhaps ~A" caller (lists->string form (copy form (make-list 3))))
			   
			   (when (symbol? end)
			     (let ((v (var-member end env)))
			       (if (and (var? v)
					(equal? `(string-length ,str) (var-initial-value v))
					(not (any? (lambda (p)
						     (set!? p env))
						   (var-history v))))   ;  if len is still (string-length x), (substring x 1 len) -> (substring x 1)
				   (lint-format "perhaps, if ~A is still ~A, ~A" caller end (var-initial-value v)
						(lists->string form (copy form (make-list 3))))))))))))))
	 
	 (hash-table-set! h 'substring sp-substring))
	
	;; ---------------- list, *vector ----------------
	(let ((seq-maker (lambda (seq)
			   (cdr (assq seq '((list . make-list) 
					    (vector . make-vector)
					    (float-vector . make-float-vector)
					    (int-vector . make-int-vector)
					    (byte-vector . make-byte-vector))))))
	      (seq-default (lambda (seq)
			     (cdr (assq seq '((list . #f) 
					      (vector . #<unspecified>)
					      (float-vector . 0.0)
					      (int-vector . 0)
					      (byte-vector . 0)))))))
	  (define (sp-list caller head form env)
	    (let ((len (length form))
		  (val (and (pair? (cdr form))
			    (cadr form))))
	      (when (and (> len 2)
			 (every? (lambda (a) (equal? a val)) (cddr form)))
		(if (code-constant? val)
		    (if (> len 4)                     ; (vector 12 12 12 12 12 12) -> (make-vector 6 12)
			(lint-format "perhaps ~A~A" caller
				     (lists->string form 
						    (if (eqv? (seq-default head) val)
							`(,(seq-maker head) ,(- len 1))
							`(,(seq-maker head) ,(- len 1) ,val)))
				     (if (and (sequence? val)
					      (not (null? val)))
					 (format #f "~%~NCor wrap (copy ~S) in a function and call that ~A times"
						 lint-left-margin #\space
						 val (- len 1))
					 "")))
		    (if (pair? val)
			(if (or (side-effect? val env)
				(hash-table-ref makers (car val)))
			    (if (> (tree-leaves val) 2)
				;; I think we need to laboriously repeat the function call here:
				;;    (let ((a 1) (b 2) (c 3)) 
				;;      (define f (let ((ctr 0)) (lambda (x y z) (set! ctr (+ ctr 1)) (+ x y ctr (* 2 z)))))
				;;      (list (f a b c) (f a b c) (f a b c) (f a b c))
				;; so (apply list (make-list 4 (_1_))) or variants thereof fail
				;;   (eval (append '(list) (make-list 4 '(_1_))))
				;; works, but it's too ugly.
				(lint-format "perhaps ~A" caller
					     (lists->string form 
							    `(let ((_1_ (lambda () ,val)))
							       (,head ,@(make-list (- len 1) '(_1_)))))))
			    ;; if seq copy else
			    (lint-format "perhaps ~A" caller  ; (vector (car x) (car x) (car x) (car x)) -> (make-vector 4 (car x))
					 (lists->string  form `(,(seq-maker head) ,(- len 1) ,val)))))))))

	  (for-each (lambda (f) (hash-table-set! h f sp-list)) '(list vector int-vector float-vector byte-vector)))

	;; ---------------- list-tail ----------------
	(let ()
	  (define (sp-list-tail caller head form env)
	    (if (= (length form) 3)
		(if (eqv? (caddr form) 0)                     ; (list-tail x 0) -> x
		    (lint-format "perhaps ~A" caller (lists->string form (cadr form)))
		    (if (and (pair? (cadr form))
			     (eq? (caadr form) 'list-tail))
			(lint-format "perhaps ~A" caller      ; (list-tail (list-tail x 1) 2) -> (list-tail x 3)
				     (lists->string form 
						    (if (and (integer? (caddr form))
							     (integer? (caddr (cadr form))))
							`(list-tail ,(cadadr form) ,(+ (caddr (cadr form)) (caddr form)))
							`(list-tail ,(cadadr form) (+ ,(caddr (cadr form)) ,(caddr form))))))))))
	  (hash-table-set! h 'list-tail sp-list-tail))
	
	;; ---------------- eq? ----------------
	(let ()
	  (define (sp-eq? caller head form env)
	    (if (< (length form) 3)  ; (eq?)
		(lint-format "eq? needs 2 arguments: ~A" caller (truncated-list->string form))
		(let* ((arg1 (cadr form))
		       (arg2 (caddr form))
		       (eq1 (eqf arg1 env))
		       (eq2 (eqf arg2 env))
		       (specific-op (and (eq? (cadr eq1) (cadr eq2))
					 (not (memq (cadr eq1) '(eqv? equal?)))
					 (cadr eq1))))
		  
		  (eval-constant-expression caller form)
		  
		  (if (or (eq? (car eq1) 'equal?)
			  (eq? (car eq2) 'equal?))         ; (eq? #(0) #(0))
		      (lint-format "eq? should be equal?~A in ~S" caller (if specific-op (format #f " or ~A" specific-op) "") form)
		      (if (or (eq? (car eq1) 'eqv?)
			      (eq? (car eq2) 'eqv?))       ; (eq? x 1.5)
			  (lint-format "eq? should be eqv?~A in ~S" caller (if specific-op (format #f " or ~A" specific-op) "") form)))
		  
		  (let ((expr 'unset))
		    (cond ((or (not arg1)                  ; (eq? #f x) -> (not x)
			       (quoted-not? arg1))
			   (set! expr (simplify-boolean `(not ,arg2) () () env)))
			  
			  ((or (not arg2)                  ; (eq? x #f) -> (not x)
			       (quoted-not? arg2))
			   (set! expr (simplify-boolean `(not ,arg1) () () env)))
			  
			  ((and (any-null? arg1)           ; (eq? () x) -> (null? x)
				(not (code-constant? arg2)))
			   (set! expr (or (equal? arg2 '(list)) ; (eq? () (list)) -> #t
					  `(null? ,arg2))))
			  
			  ((and (any-null? arg2)           ; (eq? x ()) -> (null? x)
				(not (code-constant? arg1)))
			   (set! expr (or (equal? arg1 '(list))
					  `(null? ,arg1))))
			  
			  ((and (eq? arg1 #t)              ; (eq? #t <boolean-expr>) -> boolean-expr
				(pair? arg2)
				(eq? (return-type (car arg2) env) 'boolean?))
			   (set! expr arg2))
			  
			  ((and (eq? arg2 #t)              ; (eq? <boolean-expr> #t) -> boolean-expr
				(pair? arg1)
				(eq? (return-type (car arg1) env) 'boolean?))
			   (set! expr arg1)))
		    
		    (if (not (eq? expr 'unset))            ; (eq? x '()) -> (null? x)
			(lint-format "perhaps ~A" caller (lists->string form expr)))))))
	  (hash-table-set! h 'eq? sp-eq?))
	
	;; ---------------- eqv? equal? ----------------
	(let ()
	  (define (sp-eqv? caller head form env)
	    (define (useless-copy? a)
	      (and (pair? a)
		   (memq (car a) '(copy string-copy vector-copy list-copy))
		   (null? (cddr a))))
	    (if (< (length form) 3)
		(lint-format "~A needs 2 arguments: ~A" caller head (truncated-list->string form))
		(let* ((arg1 (cadr form))
		       (arg2 (caddr form))
		       (eq1 (eqf arg1 env))
		       (eq2 (eqf arg2 env))
		       (specific-op (and (eq? (cadr eq1) (cadr eq2))
					 (not (memq (cadr eq1) '(eq? eqv? equal?)))
					 (cadr eq1))))
		  
		  (eval-constant-expression caller form)

		  (if (or (useless-copy? arg1)
			  (useless-copy? arg2))       ; (equal? (vector-copy #(a b c)) #(a b c)) -> (equal? #(a b c) #(a b c))
		      (lint-format "perhaps ~A" caller
				   (lists->string form
						  `(,head ,(if (useless-copy? arg1) (cadr arg1) arg1)
							  ,(if (useless-copy? arg2) (cadr arg2) arg2)))))
		  (if (and (string? (cadr form))
			   (= (length (cadr form)) 1))
		      (let ((s2 (caddr form)))
			(if (pair? s2)
			    (if (eq? (car s2) 'string)          ; (equal? "[" (string r)) -> (char=? #\[ r)
				(lint-format "perhaps ~A" caller 
					     (lists->string form `(char=? ,(string-ref (cadr form) 0) ,(cadr s2))))
				(if (and (eq? (car s2) 'substring)
					 (= (length s2) 4)      ; (equal? "^" (substring s 0 1)) -> (char=? #\^ (string-ref s 0))
					 (eqv? (list-ref s2 2) 0)
					 (eqv? (list-ref s2 3) 1))
				    (lint-format "perhaps ~A" caller
						 (lists->string form `(char=? ,(string-ref (cadr form) 0) (string-ref ,(cadr s2) 0)))))))))

		  (if (and (not (eq? (cadr eq1) (cadr eq2)))    ; (eqv? ":" (string-ref s 0))
			   (memq (cadr eq1) '(char=? string=?))
			   (memq (cadr eq2) '(char=? string=?)))
		      (lint-format "this can't be right: ~A" caller form))
			
		  ;; (equal? a (list b)) and equivalents happens a lot, but is the extra consing worse than 
		  ;;    (and (pair? a) (equal? (car a) b) (null? (cdr a))) -- code readability seems more important here
		  
		  (cond ((or (eq? (car eq1) 'equal?)
			     (eq? (car eq2) 'equal?))
			 (if (eq? head 'equal?)
			     (if specific-op                    ; equal? could be string=? in (equal? (string x) (string-append y z))
				 (lint-format "~A could be ~A in ~S" caller head specific-op form))
			     (lint-format "~A should be equal?~A in ~S" caller head 
					  (if specific-op (format #f " or ~A" specific-op) "") 
					  form)))
			
			((or (eq? (car eq1) 'eqv?)
			     (eq? (car eq2) 'eqv?))
			 (if (eq? head 'eqv?)
			     (if specific-op                    ; (eqv? (integer->char x) #\null)
				 (lint-format "~A could be ~A in ~S" caller head specific-op form))
				 (lint-format "~A ~A be eqv?~A in ~S" caller head 
					      (if (eq? head 'eq?) "should" "could") 
					      (if specific-op (format #f " or ~A" specific-op) "")
					      form)))
			
			((not (or (eq? (car eq1) 'eq?)
				  (eq? (car eq2) 'eq?))))

			((not (and arg1 arg2))                  ; (eqv? x #f) -> (not x)
			 (lint-format "~A could be not: ~A" caller head (lists->string form `(not ,(or arg1 arg2)))))
			
			((or (any-null? arg1) 
			     (any-null? arg2))                  ; (eqv? x ()) -> (null? x)
			 (lint-format "~A could be null?: ~A" caller head
				      (lists->string form 
						     (if (any-null? arg1)
							 `(null? ,arg2)
							 `(null? ,arg1)))))
			(else                                   ; (eqv? x 'a)
			 (lint-format "~A could be eq?~A in ~S" caller head 
				      (if specific-op (format #f " or ~A" specific-op) "") 
				      form))))))
	  (hash-table-set! h 'eqv? sp-eqv?)
	  (hash-table-set! h 'equal? sp-eqv?))
	
	;; ---------------- map for-each ----------------
	(let ()
	  (define (sp-map caller head form env)
	    (let* ((len (length form))
		   (args (- len 2)))
	      (if (< len 3)                           ; (map (lambda (v) (vector-ref v 0)))
		  (lint-format "~A missing argument~A in: ~A"
			       caller head 
			       (if (= len 2) "" "s") 
			       (truncated-list->string form))
		  (let ((func (cadr form))
			(ary #f))

		    ;; if zero or one args, the map/for-each is either a no-op or a function call
		    (if (any? any-null? (cddr form))  ; (map abs ())
			(lint-format "this ~A has no effect (null arg)" caller (truncated-list->string form))
			(if (and (not (tree-memq 'values form)) ; e.g. flatten in s7.html
				 (any? (lambda (p)
				    (and (pair? p)
					 (case (car p)
					   ((quote)
					    (and (pair? (cadr p))
						 (null? (cdadr p))))
					   ((list)
					    (null? (cddr p)))
					   ((cons)
					    (any-null? (caddr p)))
					   (else #f))))
				  (cddr form)))    ; (for-each display (list a)) -> (display a)
			    (lint-format "perhaps ~A" caller
					 (lists->string form
							(let ((args (map (lambda (a)
									   (if (pair? a)
									       (case (car a)
										 ((list cons)
										  (cadr a))       ; slightly inaccurate
										 ((quote)
										  (caadr a))
										 (else `(,a 0))) ; not car -- might not be a list
									       `(,a 0)))         ;   but still not right -- arg might be a hash-table
									 (cddr form))))
							  (if (eq? head 'for-each)
							      `(,(cadr form) ,@args)
							      `(list (,(cadr form) ,@args))))))))
		    ;; 2 happens a lot, but introduces evaluation order quibbles
		    ;;   we used to check for values if list arg -- got 4 hits!

		    (if (and (symbol? func)
			     (procedure? (symbol->value func *e*)))
			(begin
			  (set! ary (arity (symbol->value func *e*)))
			  (if (and (eq? head 'map)
				   (hash-table-ref no-side-effect-functions func)
				   (= len 3)
				   (pair? (caddr form))
				   (or (eq? (caaddr form) 'quote)
				       (and (eq? (caaddr form) 'list)
					    (every? code-constant? (cdaddr form)))))
			      (catch #t
				(lambda ()          ; (map symbol->string '(a b c d)) -> '("a" "b" "c" "d")
				  (let ((val (eval form)))
				    (lint-format "perhaps ~A" caller (lists->string form (list 'quote val)))))
				(lambda args #f))))
			
			(when (and (pair? func)
				   (memq (car func) '(lambda lambda*)))
			  (if (pair? (cadr func))
			      (let ((arglen (length (cadr func))))
				(set! ary (if (eq? (car func) 'lambda)
					      (if (negative? arglen) 
						  (cons (abs arglen) 512000)
						  (cons arglen arglen))
					      (cons 0 (if (or (negative? arglen)
							      (memq :rest (cadr func)))
							  512000 arglen))))))
			  (if (= len 3)
			      (let ((body (cddr func)))       ; (map (lambda (a) #f) x) -> (make-list (abs (length x)) #f)
				(if (and (null? (cdr body))
					 (code-constant? (car body)))
				    (lint-format "perhaps ~A" caller
						 (lists->string form
								`(make-list (abs (length ,(caddr form))) ,(car body)))))))))
		    (if (pair? ary)
			(if (< args (car ary))                ; (map (lambda (a b) a) '(1 2))
			    (lint-format "~A has too few arguments in: ~A"
					 caller head 
					 (truncated-list->string form))
			    (if (> args (cdr ary))            ; (map abs '(1 2) '(3 4))
				(lint-format "~A has too many arguments in: ~A"
					     caller head 
					     (truncated-list->string form)))))
		    (for-each 
		     (lambda (obj)
		       (if (and (pair? obj)
				(memq (car obj) '(vector->list string->list let->list)))
			   (lint-format* caller               ; (vector->list #(1 2)) could be simplified to: #(1 2)
					 (truncated-list->string obj) 
					 " could be simplified to: "
					 (truncated-list->string (cadr obj))
					 (string-append " ; (" (symbol->string head) " accepts non-list sequences)"))))
		     (cddr form))
		    
		    (when (eq? head 'map)
		      (when (and (memq func '(char-downcase char-upcase))
				 (pair? (caddr form))         ; (map char-downcase (string->list str)) -> (string->list (string-downcase str))
				 (eq? (caaddr form) 'string->list))
			(lint-format "perhaps ~A" caller (lists->string form `(string->list (,(if (eq? func 'char-upcase) 'string-upcase 'string-downcase) 
											     ,(cadr (caddr form)))))))
		      (when (identity? func) ; to check f here as var is more work ; (map (lambda (x) x) lst) -> lst
			(lint-format "perhaps ~A" caller (lists->string form (caddr form)))))
		    
		    (let ((arg1 (caddr form)))
		      (when (and (pair? arg1)
				 (memq (car arg1) '(cdr cddr cdddr cddddr list-tail))
				 (pair? (cdr arg1))
				 (pair? (cadr arg1))
				 (memq (caadr arg1) '(string->list vector->list)))
			(let ((string-case (eq? (caadr arg1) 'string->list))
			      (len-diff (if (eq? (car arg1) 'list-tail)
					    (caddr arg1)
					    (cdr-count (car arg1))))) ; (cdr (vector->list v)) -> (make-shared-vector v (- (length v) 1) 1)
			  (lint-format "~A accepts ~A arguments, so perhaps ~A" caller head 
				       (if string-case 'string 'vector)
				       (lists->string arg1 (if string-case
							       `(substring ,(cadadr arg1) ,len-diff)
							       `(make-shared-vector ,(cadadr arg1) (- (length ,(cadadr arg1)) ,len-diff) ,len-diff)))))))
		    (when (and (eq? head 'for-each)
			       (pair? (cadr form))
			       (eq? (caadr form) 'lambda)
			       (pair? (cdadr form))                    ; (for-each (lambda (x) (+ (abs x) 1)) lst)
			       (not (any? (lambda (x) (side-effect? x env)) (cddadr form))))
		      (lint-format "pointless for-each: ~A" caller (truncated-list->string form)))
		    
		    (when (= args 1)
		      (let ((seq (caddr form)))
			
			(when (pair? seq)
			  (case (car seq)
			    ((cons)                           ; (for-each display (cons msgs " "))
			     (if (and (pair? (cdr seq))
				      (pair? (cddr seq))
				      (code-constant? (caddr seq)))
				 (lint-format "~A will ignore ~S in ~A" caller head (caddr seq) seq)))

			    ((map)
			     (when (= (length seq) 3)
			       ;; a toss-up -- probably faster to combine funcs here, and easier to read?
			       ;;   but only if first arg is only used once in first func, and everything is simple (one-line or symbol)
			       (let* ((seq-func (cadr seq))
				      (arg-name (find-unique-name func seq-func)))
				 
				 (if (symbol? func)            ; (map f (map g h)) -> (map (lambda (_1_) (f (g _1_))) h) -- dubious
				     (if (symbol? seq-func)              
					 (lint-format "perhaps ~A" caller 
						      (lists->string form `(,head (lambda (,arg-name) 
										    (,func (,seq-func ,arg-name))) 
										  ,(caddr seq))))
					 (if (simple-lambda? seq-func)   
					     ;; (map f (map (lambda (x) (g x)) h)) -> (map (lambda (x) (f (g x))) h)
					     (lint-format "perhaps ~A" caller 
							  (lists->string form `(,head (lambda (,arg-name)
											(,func ,(tree-subst arg-name (caadr seq-func) (caddr seq-func))))
										      ,(caddr seq))))))
				     (if (less-simple-lambda? func)
					 (if (symbol? seq-func)          
					     ;; (map (lambda (x) (f x)) (map g h)) -> (map (lambda (x) (f (g x))) h)
					     (lint-format "perhaps ~A" caller 
							  (lists->string form `(,head (lambda (,arg-name)
											,@(tree-subst (list seq-func arg-name) (caadr func) (cddr func)))
										      ,(caddr seq))))
					     (if (simple-lambda? seq-func) 
						 ;; (map (lambda (x) (f x)) (map (lambda (x) (g x)) h)) -> (map (lambda (x) (f (g x))) h)
						 (lint-format "perhaps ~A" caller  
							      (lists->string form `(,head (lambda (,arg-name)
											    ,@(tree-subst (tree-subst arg-name (caadr seq-func) (caddr seq-func))
													  (caadr func) (cddr func)))
											  ,(caddr seq)))))))))))))
			;; repetitive code...
			(when (eq? head 'for-each) ; args = 1 above  ; (for-each display (list a)) -> (format () "~A" a)
			  (let ((func (cadr form)))
			    (if (memq func '(display write newline write-char write-string))
				(lint-format "perhaps ~A" caller
					     (if (and (pair? seq)
						      (memq (car seq) '(list quote)))
						 (let ((op (if (eq? func 'write) "~S" "~A"))
						       (len (- (length seq) 1)))
						   (lists->string form `(format () ,(do ((i 0 (+ i 1))
											 (str ""))
											((= i len) str)
										      (set! str (string-append str op)))
										,@(cdr seq))))
						 (let ((op (if (eq? func 'write) "~{~S~}" "~{~A~}")))
						   (lists->string form `(format () ,op ,seq)))))
				(when (and (pair? func)
					   (eq? (car func) 'lambda))
				  (let ((body (cddr func)))
				    (let ((op (write-port (car body)))
					  (larg (and (pair? (cadr func))
						     (caadr func))))
				      (when (and (symbol? larg)
						 (null? (cdadr func)) ; just one arg (one sequence to for-each) for now
						 (every? (lambda (x)
							   (and (pair? x)
								(memq (car x) '(display write newline write-char write-string))
								(or (eq? (car x) 'newline)
								    (eq? (cadr x) larg)
								    (string? (cadr x))
								    (eqv? (cadr x) #\space)
								    (and (pair? (cadr x))
									 (pair? (cdadr x))
									 (eq? (caadr x) 'number->string)
									 (eq? (cadadr x) larg)))
								(eq? (write-port x) op)))
							 body))
					;; (for-each (lambda (x) (display x) (write-char #\space)) msg)
					;; (for-each (lambda (elt) (display elt)) lst)
					(let ((ctrl-string "")
					      (arg-ctr 0))
					  
					  (define* (gather-format str (arg :unset))
					    (set! ctrl-string (string-append ctrl-string str)))
					  
					  (for-each
					   (lambda (d)
					     (if (or (memq larg d) 
						     (and (pair? (cdr d))
							  (pair? (cadr d))
							  (memq larg (cadr d))))
						 (set! arg-ctr (+ arg-ctr 1)))
					     (gather-format (display->format d)))
					   body)
					  
					  (when (= arg-ctr 1)  ; (for-each (lambda (x) (display x)) args) -> (format () "~{~A~}" args)
					    (lint-format "perhaps ~A" caller 
							 (lists->string form `(format ,op ,(string-append "~{" ctrl-string "~}") ,seq)))))))))
				)))))))))
	  (for-each (lambda (f)
		      (hash-table-set! h f sp-map))
		    '(map for-each)))
	
	;; ---------------- magnitude ----------------
	(hash-table-set! 
	 h 'magnitude 
	 (lambda (caller head form env)
	   (if (and (= (length form) 2)  ;  (magnitude 2/3)
		    (memq (->lint-type (cadr form)) '(integer? rational? real?)))
	       (lint-format "perhaps use abs here: ~A" caller form))))

	;; (hash-table-set! h 'modulo (lambda (caller head form env) (format *stderr* "~A~%" form)))
	;; (modulo (- 512 (modulo offset 512)) 512)
	;; (modulo (char->integer (string-ref seed j)) 255)


	;; ---------------- open-input-file open-output-file ----------------
	(let ()
	  (define (sp-open-input-file caller head form env)
	    (if (and (pair? (cdr form))
		     (pair? (cddr form))
		     (string? (caddr form))        ; (open-output-file x "fb+")
		     (not (memv (string-ref (caddr form) 0) '(#\r #\w #\a)))) ; b + then e m c x if gcc
		(lint-format "unexpected mode: ~A" caller form)))
	  (for-each (lambda (f)
		      (hash-table-set! h f sp-open-input-file))
		    '(open-input-file open-output-file)))
	
	;; ---------------- values ----------------
	(let ()
	  (define (sp-values caller head form env)
	    (cond ((member 'values (cdr form) (lambda (a b)
						(and (pair? b)   ;  (values 2 (values 3 4) 5) -> (values 2 3 4 5)
						     (eq? (car b) 'values))))
		   (lint-format "perhaps ~A" caller (lists->string form `(values ,@(splice-if (lambda (x) (eq? x 'values)) (cdr form))))))
		  ((= (length form) 2)
		   (lint-format "perhaps ~A" caller 
				(lists->string form              ;  (values ({list} 'x ({apply_values} y))) -> (cons 'x y)
					       (if (and (pair? (cadr form))
							(eq? (caadr form) #_{list})
							(not (qq-tree? (cadr form))))
						   (un_{list} (cadr form))
						   (cadr form)))))
		  ((and (assq #_{list} (cdr form))
			(not (any? (lambda (a)
				     (and (pair? a)
					  (memq (car a) '(#_{list} #_{apply_values}))
					  (qq-tree? a)))
				   (cdr form))))
		   (lint-format "perhaps ~A" caller
				(lists->string form              ;  (values ({list} 'x y) a) -> (values (list 'x y) a)
					       `(values ,@(map (lambda (a)
								 (if (and (pair? a)
									  (eq? (car a) #_{list}))
								     (un_{list} a)
								     a))
							       (cdr form))))))))
	  (hash-table-set! h 'values sp-values))
	
	;; ---------------- call-with-values ----------------
	(let ()
	 (define (sp-call/values caller head form env)  ; (call/values p c) -> (c (p))
	   (when (= (length form) 3)
	     (let ((producer (cadr form))
		   (consumer (caddr form)))
	       (let* ((produced-values (mv-range producer env))
		      (consumed-values (and produced-values
					    (or (and (symbol? consumer)
						     (arg-arity consumer env))
						(and (pair? consumer)
						     (eq? (car consumer) 'lambda)
						     (pair? (cadr consumer))
						     (let ((len (length (cadr consumer))))
						       (if (negative? len)
							   (cons (abs len) (cdr (arity +))) ; 536870912 = MAX_ARITY in s7.c
							   (cons len len))))))))
		 (if (and consumed-values
			  (or (> (car consumed-values) (car produced-values))
			      (< (cdr consumed-values) (cadr produced-values))))
		     (let ((clen ((if (> (car consumed-values) (car produced-values)) car cdr) consumed-values)))
		       (lint-format "call-with-values consumer ~A wants ~D value~P, but producer ~A returns ~A" 
				    caller
				    (truncated-list->string consumer)
				    clen clen
				    (truncated-list->string producer)
				    ((if (> (car consumed-values) (car produced-values)) car cadr) produced-values)))))
	       
	       (cond ((not (pair? producer))             ;  (call-with-values log c)
		      (if (and (symbol? producer)
			       (not (memq (return-type producer ()) '(#t #f values))))
			  (lint-format "~A does not return multiple values" caller producer)
			  (lint-format "perhaps ~A" caller (lists->string form `(,consumer (,producer))))))
		     
		     ((not (eq? (car producer) 'lambda)) ; (call-with-values (eval p env) (eval c env)) -> ((eval c env) ((eval p env)))
		      (lint-format "perhaps ~A" caller (lists->string form `(,consumer (,producer)))))
		     
		     ((pair? (cadr producer))            ; (call-with-values (lambda (x) 0) list)
		      (lint-format "~A requires too many arguments" caller (truncated-list->string producer)))
		     
		     ((symbol? (cadr producer))          ; (call-with-values (lambda x 0) list)
		      (lint-format "~A's parameter ~A will always be ()" caller (truncated-list->string producer) (cadr producer)))
		     
		     ((and (pair? (cddr producer))       ; (call-with-values (lambda () (read-char p)) cons)
			   (null? (cdddr producer)))     ; (call-with-values (lambda () (values 1 2 3)) list) -> (list 1 2 3)
		      (let ((body (caddr producer)))
			(if (or (code-constant? body)
				(and (pair? body)
				     (symbol? (car body))
				     (not (memq (return-type (car body) ()) '(#t #f values)))))
			    (lint-format "~A does not return multiple values" caller body)
			    (lint-format "perhaps ~A" caller 
					 (lists->string form 
							(if (and (pair? body)
								 (eq? (car body) 'values))
							    `(,consumer ,@(cdr body))
							    `(,consumer ,body)))))))
		     
		     (else (lint-format "perhaps ~A" caller (lists->string form `(,consumer (,producer)))))))))
	 (hash-table-set! h 'call-with-values sp-call/values))
	
	;; ---------------- multiple-value-bind ----------------
	(let ()
	 (define (sp-mvb caller head form env)
	   (when (>= (length form) 4)
	     (let ((vars (cadr form))
		   (producer (caddr form))
		   (body (cdddr form)))
	       
	       (if (null? vars)
		   (lint-format "this multiple-value-bind is pointless; perhaps ~A" caller
				(lists->string form 
					       (if (side-effect? producer env)
						   `(begin ,producer ,@body)
						   (if (null? (cdr body))
						       (car body)
						       `(begin ,@body)))))
		   
		   (unless (symbol? vars)               ; else any number of values is ok
		     (let ((vals (mv-range producer env))      ;  (multiple-value-bind (a b) (values 1 2 3) b)
			   (args (length vars)))
		       (if (and (pair? vals)
				(not (<= (car vals) args (cadr vals))))
			   (lint-format "multiple-value-bind wants ~D values, but ~A returns ~A" 
					caller args 
					(truncated-list->string producer)
					((if (< args (car vals)) car cadr) vals)))
		       
		       (if (and (pair? producer)               ;  (multiple-value-bind (a b) (f) b) -> ((lambda (a b) b) (f))
				(symbol? (car producer))
				(not (memq (return-type (car producer) ()) '(#t #f values))))
			   (lint-format "~A does not return multiple values" caller (car producer))
			   (lint-format "perhaps ~A" caller 
					    (lists->string form 
							   (if (and (null? (cdr body))
								    (pair? (car body))
								    (equal? vars (cdar body))
								    (defined? (caar body))
								    (equal? (arity (symbol->value (caar body))) (cons args args)))
							       `(,(caar body) ,producer)
							       `((lambda ,vars ,@body) ,producer)))))))))))
	 (hash-table-set! h 'multiple-value-bind sp-mvb))
	
	;; ---------------- let-values ----------------
	(let ()
	 (define (sp-let-values caller head form env)
	   (if (and (pair? (cdr form))
		    (pair? (cadr form)))
	       (if (null? (cdadr form)) ; just one set of vars
		   (let ((call (caadr form)))
		     (if (and (pair? call)
			      (pair? (cdr call)))
			 (lint-format "perhaps ~A" caller      ;  (let-values (((x) (values 1))) x) -> ((lambda (x) x) (values 1))
				      (lists->string form
						     `((lambda ,(car call)
							 ,@(cddr form))
						       ,(cadr call))))))
		   (if (every? pair? (cadr form))
		       (lint-format "perhaps ~A" caller        ;  (let-values (((x) (values 1)) ((y) (values 2))) (list x y)) ...
				    (lists->string 
				     form
				     `(with-let 
					  (apply sublet (curlet) 
						 (list ,@(map (lambda (v)
								`((lambda ,(car v)
								    (values ,@(map (lambda (name)
										     (values (symbol->keyword name) name))
										   (args->proper-list (car v)))))
								  ,(cadr v)))
							      (cadr form))))
					,@(cddr form))))))))
	 (hash-table-set! h 'let-values sp-let-values))
	
	;; ---------------- let*-values ----------------
	(hash-table-set! 
	 h 'let*-values 
	 (lambda (caller head form env)
	   (if (and (pair? (cdr form))
		    (pair? (cadr form)))
	       (lint-format "perhaps ~A" caller
			    (lists->string form               ;   (let*-values (((a) (f x))) (+ a b)) -> (let ((a (f x))) (+ a b))
					   (let loop ((var-data (cadr form)))
					     (let ((v (car var-data)))
					       (if (and (pair? (car v))  ; just one var
							(null? (cdar v)))
						   (if (null? (cdr var-data))
						       `(let ((,(caar v) ,(cadr v))) ,@(cddr form))
						       `(let ((,(caar v) ,(cadr v))) ,(loop (cdr var-data))))
						   (if (null? (cdr var-data))
						       `((lambda ,(car v) ,@(cddr form)) ,(cadr v))
						       `((lambda ,(car v) ,(loop (cdr var-data))) ,(cadr v)))))))))))

	;; ---------------- define-values ----------------
	(hash-table-set! 
	 h 'define-values 
	 (lambda (caller head form env)
	   (when (pair? (cdr form))
	     (if (null? (cadr form))
		 (lint-format "~A is pointless" caller (truncated-list->string form))
		 (when (pair? (cddr form))
		   (lint-format "perhaps ~A" caller        ;  (define-values (x y) (values 3 2)) -> (varlet (curlet) ((lambda (x y) (curlet)) (values 3 2)))
				(cond ((symbol? (cadr form))
				       (lists->string form `(define ,(cadr form) (list ,(caddr form)))))
				      
				      ((and (pair? (cadr form))
					    (null? (cdadr form)))
				       (lists->string form `(define ,(caadr form) ,(caddr form))))
				      
				      (else
				       (let-temporarily ((target-line-length 120))
					 (truncated-lists->string form
								  `(varlet (curlet) 
								     ((lambda ,(cadr form) 
									(curlet)) 
								      ,(caddr form)))))))))))))
	;; ---------------- eval ----------------
	(let ()
	 (define (sp-eval caller head form env)
	   (case (length form)
	     ((2)
	      (let ((arg (cadr form)))
		(if (not (pair? arg))
		    (if (not (symbol? arg))           ;  (eval 32)
			(lint-format "this eval is pointless; perhaps ~A" caller (lists->string form arg)))
		    (case (car arg) 
		      ((quote)                        ;  (eval 'x)
		       (lint-format "perhaps ~A" caller (lists->string form (cadr arg))))

		      ((string->symbol)               ;  (eval (string->symbol "x")) -> x
		       (if (string? (cadr arg))
			   (lint-format "perhaps ~A" caller (lists->string form (string->symbol (cadr arg))))))
		      
		      ((with-input-from-string call-with-input-string)
		       (if (and (pair? (cdr arg))     ;  (eval (call-with-input-string port read)) -> (eval-string port)
				(pair? (cddr arg))
				(eq? (caddr arg) 'read))
			   (lint-format "perhaps ~A" caller (lists->string form `(eval-string ,(cadr arg))))))
		      
		      ((read)
		       (if (and (= (length arg) 2)    ;  (eval (read (open-input-string expr))) -> (eval-string expr)
				(pair? (cadr arg))
				(eq? (caadr arg) 'open-input-string))
			   (lint-format "perhaps ~A" caller (lists->string form `(eval-string ,(cadadr arg))))))

		      ((list)
		       (if (every? (lambda (p)        ;  (eval (list '* 2 x)) -> (* 2 (eval x))
				     (or (symbol? p)
					 (code-constant? p)))
				   (cdr arg))
			   (lint-format "perhaps ~A" caller
					(lists->string form
						       (map (lambda (p)
							      (if (and (pair? p)
								       (eq? (car p) 'quote))
								  (cadr p)
								  (if (code-constant? p)
								      p
								      (list 'eval p))))
							    (cdr arg))))))))))
	     ((3)
	      (let ((arg (cadr form))
		    (e (caddr form)))
		(if (and (pair? arg)
			 (eq? (car arg) 'quote))
		    (lint-format "perhaps ~A" caller   ;  (eval 'x env) -> (env 'x)
				 (lists->string form
						(if (symbol? (cadr arg))
						    `(,e ,arg)
						    `(with-let ,e ,@(unbegin (cadr arg)))))))))))
	 (hash-table-set! h 'eval sp-eval))

	;; ---------------- fill! etc ----------------
	(let ()
	  (define (sp-fill! caller head form env)
	    (if (= (length form) 5)
		(check-start-and-end caller head (cdddr form) form env)))
	  (for-each (lambda (f)
		      (hash-table-set! h f sp-fill!))
		    '(fill! string-fill! list-fill! vector-fill!)))
	
	;; ---------------- write-string ----------------
	(hash-table-set! 
	 h 'write-string 
	 (lambda (caller head form env)
	   (if (= (length form) 4)
	       (check-start-and-end caller 'write-string (cddr form) form env))))

	;; ---------------- read-line ----------------
	(hash-table-set!
	 h 'read-line
	 (lambda (caller head form env)
	   (if (and (= (length form) 3)
		    (code-constant? (caddr form))
		    (not (boolean? (caddr form))))   ;  (read-line in-port 'concat)
	       (lint-format "the third argument should be boolean (#f=default, #t=include trailing newline): ~A" caller form))))
	
	;; ---------------- string-length ----------------
	(let ()
	  (define (sp-string-length caller head form env)
	    (when (= (length form) 2)
	      (if (string? (cadr form))              ;  (string-length "asdf") -> 4
		  (lint-format "perhaps ~A -> ~A" caller (truncated-list->string form) (string-length (cadr form)))
		  (if (and (pair? (cadr form))       ;  (string-length (make-string 3)) -> 3
			   (eq? (caadr form) 'make-string))
		      (lint-format "perhaps ~A" caller (lists->string form (cadadr form)))))))

	  (hash-table-set! h 'string-length sp-string-length))
	
	;; ---------------- vector-length ----------------
	(let ()
	  (define (sp-vector-length caller head form env)
	    (when (= (length form) 2)
	      (if (vector? (cadr form))
		  (lint-format "perhaps ~A -> ~A" caller (truncated-list->string form) (vector-length (cadr form)))
		  (let ((arg (cadr form)))
		    (if (pair? arg)
			(if (eq? (car arg) 'make-vector)  ;  (vector-length (make-vector 10)) -> 10
			    (lint-format "perhaps ~A" caller (lists->string form (cadr arg)))
			    (if (memq (car arg) '(copy vector-copy))
				(lint-format "perhaps ~A" caller 
					     (lists->string form   ;   (vector-length (vector-copy arr start end)) -> (- end start)
							    (if (null? (cddr arg))
								`(vector-length ,(cadr arg))
								(if (eq? (car arg) 'copy)
								    `(vector-length ,(caddr arg))
								    (let ((start (caddr arg))
									  (end (if (null? (cdddr arg))
										   `(vector-length ,(cadr arg))
										   (cadddr arg))))
								      `(- ,end ,start)))))))))))))
	  (hash-table-set! h 'vector-length sp-vector-length))
	
	;; ---------------- dynamic-wind ----------------
	(let ()
	 (define (sp-dw caller head form env)
	   (when (= (length form) 4)
	     (let ((init (cadr form))
		   (body (caddr form))
		   (end (cadddr form))
		   (empty 0))
	       ;; (equal? init end) as a mistake doesn't seem to happen
	       
	       (when (and (pair? init)
			  (eq? (car init) 'lambda))
		 (if (not (null? (cadr init)))
		     (lint-format "dynamic-wind init function should be a thunk: ~A" caller init))
		 (if (pair? (cddr init))
		     (let ((last-expr (list-ref init (- (length init) 1))))
		       (if (not (pair? last-expr))
			   (if (null? (cdddr init))
			       (set! empty 1))
			   (unless (side-effect? last-expr env)
			     (if (null? (cdddr init))
				 (set! empty 1))    ;  (dynamic-wind (lambda () (s7-version)) (lambda () (list)) (lambda () #f))
			     (lint-format "this could be omitted: ~A in ~A" caller last-expr init))))))
	       
	       (if (and (pair? body)
			(eq? (car body) 'lambda))
		   (if (not (null? (cadr body)))
		       (lint-format "dynamic-wind body function should be a thunk: ~A" caller body))
		   (set! empty 3)) ; don't try to access body below
	       
	       (when (and (pair? end)
			  (eq? (car end) 'lambda))
		 (if (not (null? (cadr end)))
		     (lint-format "dynamic-wind end function should be a thunk: ~A" caller end))
		 (if (pair? (cddr end))
		     (let ((last-expr (list-ref end (- (length end) 1))))
		       (if (not (pair? last-expr))
			   (if (null? (cdddr end))
			       (set! empty (+ empty 1)))
			   (unless (side-effect? last-expr env) ; or if no side-effects in any (also in init)
			     (if (null? (cdddr end))
				 (set! empty (+ empty 1)))
			     (lint-format "this could be omitted: ~A in ~A" caller last-expr end)))
		       (if (= empty 2)          ;  (dynamic-wind (lambda () #f) (lambda () #()) (lambda () #f)) -> #()
			   (lint-format "this dynamic-wind is pointless, ~A" caller 
					(lists->string form (if (null? (cdddr body)) (caddr body) `(begin ,@(cddr body))))))))))))
	 (hash-table-set! h 'dynamic-wind sp-dw))
	
	;; ---------------- *s7* ----------------
	(hash-table-set! 
	 h '*s7* 
	 (let ((s7-fields (let ((h (make-hash-table)))
			    (for-each (lambda (f)
					(hash-table-set! h f #t))
				      '(print-length safety cpu-time heap-size free-heap-size gc-freed max-string-length max-list-length 
				        max-vector-length max-vector-dimensions default-hash-table-length initial-string-port-length 
					gc-protected-objects file-names rootlet-size c-types stack-top stack-size stacktrace-defaults
					max-stack-size stack catches exits float-format-precision bignum-precision default-rationalize-error 
					default-random-state morally-equal-float-epsilon hash-table-float-epsilon undefined-identifier-warnings 
					gc-stats symbol-table-locked? c-objects history-size profile-info))
			    h)))
	   (lambda (caller head form env)
	     (if (= (length form) 2)
		 (let ((arg (cadr form)))
		   (if (and (pair? arg)
			    (eq? (car arg) 'quote)
			    (symbol? (cadr arg))            ;  (*s7* 'vector-print-length)
			    (not (hash-table-ref s7-fields (cadr arg))))
		       (lint-format "unknown *s7* field: ~A" caller arg)))))))
	
	;; ---------------- throw ----------------
	(hash-table-set! 
	 h 'throw 
	 (lambda (caller head form env)
	   (if (pair? (cdr form))
	       (let* ((tag (cadr form))
		      (eq (eqf tag env)))
		 (if (not (member eq '((eq? eq?) (#t #t))))
		     (lint-format "~A tag ~S is unreliable (catch uses eq? to match tags)" caller 'throw tag))))))
	
	;; ---------------- make-hash-table ----------------
	(hash-table-set! 
	 h 'make-hash-table 
	 (lambda (caller head form env)
	   (if (= (length form) 3)
	       (let ((func (caddr form)))
		 (if (and (symbol? func)  ;  (make-hash-table eq? symbol-hash)
			  (not (memq func '(eq? eqv? equal? morally-equal? char=? char-ci=? string=? string-ci=? =))))
		     (lint-format "make-hash-table function, ~A, is not a hash function" caller func))))))
	
	;; ---------------- deprecated funcs ---------------- 
	(let ((deprecated-ops '((global-environment . rootlet)
				(current-environment . curlet)
				(make-procedure-with-setter . dilambda)
				(procedure-with-setter? . dilambda?)
				(make-random-state . random-state))))

	  (define (sp-deprecate caller head form env)  ;  (make-random-state 123 432)
	    (lint-format "~A is deprecated; use ~A" caller head (cond ((assq head deprecated-ops) => cdr))))

	  (for-each (lambda (op)
		      (hash-table-set! h (car op) sp-deprecate))
		    deprecated-ops))

	;; ---------------- eq null eqv equal ----------------
	(let ()
	  (define (sp-null caller head form env)
	    (if (not (var-member head env))          ;  (if (null (cdr x)) 0)
		(lint-format "misspelled '~A? in ~A?" caller head form)))
	  (for-each (lambda (f)
		      (hash-table-set! h f sp-null))
		    '(null eq eqv equal))) ; (null (cdr...)) 

	;; ---------------- string-index ----------------
	(let ()
	  (define (sp-string-index caller head form env)
	    (if (and (pair? (cdr form))
                     (pair? (cddr form))
                     (not (var-member 'string-index env))
		     (or (char? (caddr form))
			 (let ((sig (arg-signature (caddr form) env)))
			   (and (pair? sig)
				(eq? (car sig) 'char?)))))
		(lint-format "perhaps ~A" caller          ;  (string-index path #\/) -> (char-position #\/ path)
			     (lists->string form `(char-position ,(caddr form) ,(cadr form) ,@(cdddr form))))))
	  (hash-table-set! h 'string-index sp-string-index))

	;; ---------------- cons* ----------------
	(let ()
	  (define (sp-cons* caller head form env)
	    (unless (var-member 'cons env)
	      (case (length form)
		((2) (lint-format "perhaps ~A" caller (lists->string form (cadr form))))
		((3) (lint-format "perhaps ~A" caller 
				  (lists->string form      ;  cons* x y) -> (cons x y)
						 (if (any-null? (caddr form))
						     `(list ,(cadr form))
						     `(cons ,@(cdr form))))))
		((4) (lint-format "perhaps ~A" caller 
				  (lists->string form      ;  (cons* (symbol->string v) " | " (w)) -> (cons (symbol->string v) (cons " | " (w)))
						 (if (any-null? (cadddr form))
						     `(list ,(cadr form) ,(caddr form))
						     `(cons ,(cadr form) (cons ,@(cddr form))))))))))
	  (hash-table-set! h 'cons* sp-cons*))

	;; ---------------- the-environment etc ----------------
	(let ((other-names '((the-environment . curlet)
			     (interaction-environment . curlet)
			     (system-global-environment . rootlet)
			     (user-global-environment . rootlet)
			     (user-initial-environment . rootlet)
			     (procedure-environment . funclet)
			     (environment? . let?)
			     (environment-set! . let-set!)
			     (environment-ref . let-ref)
			     (fluid-let . let-temporarily)
			     (unquote-splicing apply values ...)
			     (bitwise-and . logand) 
			     (bitwise-ior . logior) 
			     (bitwise-xor . logxor) 
			     (bitwise-not . lognot)
			     (bit-and . logand) 
			     (bit-or . logior) 
			     (bit-xor . logxor) 
			     (bit-not . lognot)
			     (arithmetic-shift . ash)
			     (vector-for-each . for-each)
			     (string-for-each . for-each)
			     (list-copy . copy)
			     (bytevector? . byte-vector?)
			     (bytevector . byte-vector)
			     (make-bytevector . make-byte-vector)
			     (bytevector-u8-ref . byte-vector-ref)
			     (bytevector-u8-set! . byte-vector-set!)
			     (bytevector-length . length)
			     (write-bytevector . write-string)
			     (hash-set! . hash-table-set!)     ; Guile
			     (hash-ref . hash-table-ref)
			     (hashq-set! . hash-table-set!)
			     (hashq-ref . hash-table-ref)
			     (hashv-set! . hash-table-set!)
			     (hashv-ref . hash-table-ref)
			     (hash-table-get . hash-table-ref) ; Gauche
			     (hash-table-put! . hash-table-set!)
			     (hash-table-num-entries . hash-table-entries)
			     (hashtable? . hash-table?) ; Bigloo
			     (hashtable-size . hash-table-entries)
			     (hashtable-get . hash-table-ref)
			     (hashtable-put! . hash-table-set!)
			     (hash-for-each . for-each)
			     (exact-integer? . integer?)
			     (truncate-quotient . quotient)
			     (truncate-remainder . remainder)
			     (floor-remainder . modulo)
			     (read-u8 . read-byte)
			     (write-u8 . write-byte)
			     (write-simple . write)
			     (peek-u8 . peek-char)
			     (u8-ready? . char-ready?) 
			     (open-input-bytevector . open-input-string)
			     (open-output-bytevector . open-output-string)
			     (raise . error)
			     (raise-continuable . error))))

	  (define (sp-other-names caller head form env)
	    (if (not (var-member head env))
		(let ((counts (or (hash-table-ref other-names-counts head) 0)))
		  (when (< counts 2)
		    (hash-table-set! other-names-counts head (+ counts 1))
		    (lint-format "~A is probably ~A in s7" caller head (cdr (assq head other-names)))))))

	  (for-each (lambda (f)
		      (hash-table-set! h (car f) sp-other-names))
		    other-names))

	(hash-table-set! 
	 h '1+ 
	 (lambda (caller head form env)
	   (if (not (var-member '1+ env))
	       (lint-format "perhaps ~A" caller (lists->string form `(+ ,(cadr form) 1))))))

	(let ()
	  (define (sp-1- caller head form env)
	    (if (not (var-member '-1+ env))
		(lint-format "perhaps ~A" caller (lists->string form `(- ,(cadr form) 1)))))

	  (hash-table-set! h '-1+ sp-1-)
	  (hash-table-set! h '1- sp-1-))


	;; ---------------- push! pop! ----------------	
	(hash-table-set! 
	 h 'push! 
	 (lambda (caller head form env) ; not predefined
	   (if (= (length form) 3)
	       (set-set (caddr form) caller form env))))
	
	(hash-table-set! 
	 h 'pop! 
	 (lambda (caller head form env) ; also not predefined
	   (if (= (length form) 2)
	       (set-set (cadr form) caller form env))))
	
	;; ---------------- receive ----------------
	(hash-table-set! 
	 h 'receive 
	 (lambda (caller head form env) ; this definition comes from Guile
	   (if (and (> (length form) 3)
		    (not (var-member 'receive env)))
	       ((hash-table-ref h 'call-with-values) 
		caller 'call-with-values
		`(call-with-values 
		     (lambda () ,(caddr form))
		   (lambda ,(cadr form) ,@(cdddr form)))
		env))))

	;; ---------------- and=> ----------------
	(hash-table-set!
	 h 'and=>
	 (lambda (caller head form env)  ;  (and=> (ref w k) v) -> (cond ((ref w k) => v) (else #f))
	   (when (and (= (length form) 3)
		      (not (var-member 'and=> env)))
	     (lint-format "perhaps ~A" caller (lists->string form `(cond (,(cadr form) => ,(caddr form)) (else #f)))))))
	   
	;; ---------------- and-let* ----------------
	(hash-table-set! 
	 h 'and-let* 
	 (lambda (caller head form env)
	   (when (and (> (length form) 2)
		      (not (var-member 'and-let* env)))
	     (let loop ((bindings (cadr form)))
	       (cond ((pair? bindings)
		      (if (binding-ok? caller 'and-let* (car bindings) env #f)
			  (loop (cdr bindings))))
		     ((not (null? bindings))
		      (lint-format "~A variable list is not a proper list? ~S" caller 'and-let* bindings))
		     ((and (pair? (cadr form))           ; (and-let* ((x (f y))) (abs x)) -> (cond ((f y) => abs))
			   (null? (cdadr form))
			   (pair? (cddr form)))
		      (lint-format "perhaps ~A" caller 
				   (lists->string form   ;  (and-let* ((x (f y))) (abs x)) -> (cond ((f y) => abs))
						  (if (and (null? (cdddr form))
							   (pair? (caddr form))
							   (pair? (cdaddr form))
							   (null? (cddr (caddr form)))
							   (eq? (caaadr form) (cadr (caddr form))))
						      `(cond (,(cadar (cadr form)) => ,(caaddr form)))
						      `(cond (,(cadar (cadr form)) => (lambda (,(caaadr form)) ,@(cddr form)))))))))))))
	h))
    ;; end special-case-functions
    ;; ----------------------------------------
    

    (define (unused-parameter? x) #t)
    (define (unused-set-parameter? x) #t)

    
    (define (check-args caller head form checkers env max-arity)
      ;; check for obvious argument type problems
      ;; caller = overall caller, head = current caller, checkers = proc or list of procs for checking args

      (define (every-compatible? type1 type2)
	(if (symbol? type1)
	    (if (symbol? type2)
		(compatible? type1 type2)
		(and (pair? type2)                   ; here everything has to match
		     (compatible? type1 (car type2))
		     (every-compatible? type1 (cdr type2))))
	    (and (pair? type1)                       ; here any match is good
		 (or (compatible? (car type1) type2)
		     (any-compatible? (cdr type1) type2)))))
      
      (define (check-checker checker at-end)
	(if (eq? checker 'integer:real?)
	    (if at-end 'real? 'integer?)
	    (if (eq? checker 'integer:any?)
		(or at-end 'integer?)
		checker)))
    
      (define (any-checker? types arg)
	(if (and (symbol? types)
		 (not (eq? types 'values)))
	    ((symbol->value types *e*) arg)
	    (and (pair? types)
		 (or (any-checker? (car types) arg)
		     (any-checker? (cdr types) arg)))))
    
      (define (report-arg-trouble caller form head arg-number checker arg uop)
	(define (prettify-arg-number argn)
	  (if (or (not (= argn 1))
		  (pair? (cddr form)))
	      (format #f "~D " argn)
	      ""))
    	(let ((op (if (and (eq? checker 'real?)
			   (eq? uop 'number?))
		      'complex?
		      uop)))
	  (if (and (or arg (not (eq? checker 'output-port?)))
		   (not (and (eq? checker 'string?)
			     (pair? arg)
			     (eq? (car arg) 'format)))   ; don't try to untangle the format non-string case
		   (not (and (pair? arg)
			     (eq? (car arg) 'length))))  ; same for length
	      (if (and (pair? op)
		       (member checker op any-compatible?))
		  (if (and *report-sloppy-assoc*
			   (not (var-member :catch env)))
		      (lint-format* caller               ; (round (char-position #\a "asb"))
				    (string-append "in " (truncated-list->string form) ", ")
				    (string-append (symbol->string head) "'s argument " (prettify-arg-number arg-number))
				    (string-append "should be " (prettify-checker-unq checker) ", ")
				    (string-append "but " (truncated-list->string arg) " might also be " 
						   (object->string (car (remove-if (lambda (o) (any-compatible? checker o)) op))))))
		  (lint-format* caller                   ; (string-ref (char-position #\a "asb") 1)
				(string-append "in " (truncated-list->string form) ", ")
				(string-append (symbol->string head) "'s argument " (prettify-arg-number arg-number))
				(string-append "should be " (prettify-checker-unq checker) ", ")
				(string-append "but " (truncated-list->string arg) " is " (prettify-checker op)))))))
      
      (when *report-func-as-arg-arity-mismatch*
	(let ((v (var-member head env)))
	  (when (and (var? v)
		     (memq (var-ftype v) '(define define* lambda lambda*))
		     (zero? (var-set v)) ; perhaps this needs to wait for report-usage?
		     (pair? (var-arglist v)))
	    (let ((source (var-initial-value v)))
	      (when (and (pair? source)
			 (pair? (cdr source))
			 (pair? (cddr source)))
		(let ((vhead (cddr source))
		      (head-arglist (var-arglist v))
		      (arg-number 1))
		  
		  (when (pair? vhead)
		    (for-each 
		     (lambda (arg)
		       ;; only check func if head is var-member and has procedure-source (var-[initial-]value?)
		       ;;   and arg has known arity, and check only if arg(par) is car, not (for example) cadr of apply
		       
		       (let ((ari (if (symbol? arg)
				      (arg-arity arg env)
				      (and (pair? arg)
					   (eq? (car arg) 'lambda)
					   (let ((len (length (cadr arg))))
					     (and (integer? len)
						  (cons (abs len)
							(if (negative? len) 500000 len)))))))
			     (par (and (> (length head-arglist) (- arg-number 1))
				       (list-ref head-arglist (- arg-number 1)))))
			 (when (and (symbol? par)
				    (pair? ari)
				    (or (> (car ari) 0)
					(< (cdr ari) 20)))
			   
			   ;; fwalk below needs to be smart about tree walking so that
			   ;;   it does not confuse (c) in (lambda (c)...) with a call on the function c.
			   ;; check only if current parameter name is not shadowed
			   
			   (let fwalk ((sym par) (tree vhead))
			     (when (pair? tree)
			       (if (eq? (car tree) sym)
				   (let ((args (- (length tree) 1)))
				     (if (> (car ari) args)
					 (lint-format "~A's parameter ~A is passed ~A and called ~A, but ~A needs ~A argument~P" caller
						      head par 
						      (truncated-list->string arg)
						      (truncated-list->string tree)
						      (truncated-list->string arg)
						      (car ari) (car ari))
					 (if (> args (cdr ari))
					     (lint-format "~A's parameter ~A is passed ~A and called ~A, but ~A takes only ~A argument~P" caller
							  head par 
							  (truncated-list->string arg)
							  (truncated-list->string tree)
							  (truncated-list->string arg)
							  (cdr ari) (cdr ari)))))
				   (case (car tree)
				     ((let let*)
				      (if (and (pair? (cdr tree))
					       (pair? (cddr tree)))
					  (let ((vs ((if (symbol? (cadr tree)) caddr cadr) tree)))
					    (if (not (any? (lambda (a) (or (not (pair? a)) (eq? sym (car a)))) vs))
						(fwalk sym ((if (symbol? (cadr tree)) cdddr cddr) tree))))))
				     
				     ((do letrec letrec*)
				      (if (and (pair? (cdr tree))
					       (pair? (cddr tree))
					       (not (any? (lambda (a) (or (not (pair? a)) (eq? sym (car a)))) (cadr tree))))
					  (fwalk sym (cddr tree))))
				     
				     ((lambda lambda*)
				      (if (and (pair? (cdr tree))
					       (pair? (cddr tree))
					       (not (any? (lambda (a) (eq? sym a)) (args->proper-list (cadr tree)))))
					  (fwalk sym (cddr tree))))
				     
				     ((define define-constant)
				      (if (and (not (eq? sym (cadr tree)))
					       (pair? (cadr tree))
					       (not (any? (lambda (a) (eq? sym a)) (args->proper-list (cdadr tree)))))
					  (fwalk sym (cddr tree))))
				     
				     ((define* define-macro define-macro* define-expansion define-bacro define-bacro*)
				      (if (and (pair? (cdr tree))
					       (pair? (cddr tree))
					       (not (any? (lambda (a) (eq? sym a)) (args->proper-list (cdadr tree)))))
					  (fwalk sym (cddr tree))))
				     
				     ((quote) #f)
				     
				     ((case)
				      (if (and (pair? (cdr tree))
					       (pair? (cddr tree)))
					  (for-each (lambda (c) (fwalk sym (cdr c))) (cddr tree))))
				     
				     (else 
				      (if (pair? (car tree))
					  (fwalk sym (car tree)))
				      (if (pair? (cdr tree))
					  (for-each (lambda (p) (fwalk sym p)) (cdr tree))))))))))
		       
		       (set! arg-number (+ arg-number 1)))
		     (cdr form)))))))))
      
      (when (pair? checkers)
	(let ((arg-number 1)
	      (flen (- (length form) 1)))
	  (call-with-exit
	   (lambda (done)
	     (for-each 
	      (lambda (arg)
		(let ((checker (check-checker (if (pair? checkers) (car checkers) checkers) (= arg-number flen))))
		  ;; check-checker only fixes up :at-end cases

		  (define (check-arg expr)
		    (unless (symbol? expr)
		      (let ((op (->lint-type expr)))
			(if (not (or (memq op '(#f #t values))
				     (every-compatible? checker op)))
			    (report-arg-trouble caller form head arg-number checker expr op)))))

		  (define (check-cond-arg expr)
		    (unless (symbol? expr)
		      (let ((op (->lint-type expr)))
			(when (pair? op)
			  (set! op (remove 'boolean? op)) ; this is for cond test, no result -- returns test if not #f, so it can't be #f!
			  (if (null? (cdr op))
			      (set! op (car op))))
			(if (not (or (memq op '(#f #t values))
				     (every-compatible? checker op)))
			    (report-arg-trouble caller form head arg-number checker expr op)))))
		  
		  ;; special case checker?
		  (if (and (symbol? checker)
			   (not (memq checker '(unused-parameter? unused-set-parameter?)))
			   (not (hash-table-ref built-in-functions checker)))
		      (let ((chk (symbol->value checker)))
			(if (and (procedure? chk)
				 (equal? (arity chk) '(2 . 2)))
			    (catch #t
			      (lambda ()
				(let ((res (chk form arg-number)))
				  (set! checker #t)
				  (if (symbol? res)
				      (set! checker res)
				      (if (string? res)
					  (lint-format "~A's argument, ~A, should be ~A" caller head arg res)))))
			      (lambda (type info)
				(set! checker #t))))))
		  
		  (if (and (pair? arg)
			   (pair? (car arg)))
		      (let ((rtn (return-type (caar arg) env)))
			(if (memq rtn '(boolean? real? integer? rational? number? complex? float? keyword? symbol? null? char?))
			    (lint-format* caller    ;  (cons ((pair? x) 2) y)
					  (string-append (symbol->string head) "'s argument ")
					  (string-append (truncated-list->string arg) " looks odd: ")
					  (string-append (object->string (caar arg)) " returns " (symbol->string rtn))
					  " which is not applicable"))))
		  
		  (when (or (pair? checker)
			    (symbol? checker)) ; otherwise ignore type check on this argument (#t -> anything goes)
		    (if arg
			(if (eq? checker 'unused-parameter?)
			    (lint-format* caller    ;  (define (f5 a . b) a) (f5 1 2)
					  (string-append (symbol->string head) "'s parameter " (number->string arg-number))
					  " is not used, but a value is passed: "
					  (truncated-list->string arg))
			    (if (eq? checker 'unused-set-parameter?)
				(lint-format* caller  ;  (define (f21 x y) (set! x 3) (+ y 1)) (f21 (+ z 1) z)
					      (string-append (symbol->string head) "'s parameter " (number->string arg-number))
					      "'s value is not used, but a value is passed: "
					      (truncated-list->string arg)))))
		    (if (not (pair? arg))
			(let ((val (cond ((not (symbol? arg))
					  arg)
					 ((constant? arg)
					  (symbol->value arg))
					 ((and (hash-table-ref built-in-functions arg)
					       (not (var-member :with-let env))
					       (not (var-member arg env)))
					  (symbol->value arg *e*))
					 (else arg))))
			  (if (not (or (and (symbol? val)
					    (not (keyword? val)))
				       (any-checker? checker val)))
			      (let ((op (->lint-type val)))
				(unless (memq op '(#f #t values))
				  (report-arg-trouble caller form head arg-number checker arg op)))))
			
			(case (car arg) 
			  ((quote)   ; '1 -> 1
			   (let ((op (if (pair? (cadr arg)) 'list? 
					 (if (symbol? (cadr arg))
					     'symbol?
					     (->lint-type (cadr arg))))))
			     ;; arg is quoted expression
			     (if (not (or (memq op '(#f #t values))
					  (every-compatible? checker op)))
				 (report-arg-trouble caller form head arg-number checker arg op))))
			  
			  ;; arg is an expression
			  ((begin let let* letrec letrec* with-let)
			   (check-arg (and (pair? (cdr arg))
					   (list-ref arg (- (length arg) 1)))))
			  
			  ((if)
			   (if (and (pair? (cdr arg))
				    (pair? (cddr arg)))
			       (let ((t (caddr arg))
				     (f (if (pair? (cdddr arg)) (cadddr arg))))
				 (check-arg t)
				 (when (and f (not (symbol? f)))
				   (check-arg f)))))
			  
			  ((dynamic-wind catch)
			   (if (= (length arg) 4)
			       (let ((f (caddr arg)))
				 (if (and (pair? f)
					  (eq? (car f) 'lambda))
				     (let ((len (length f)))
				       (if (> len 2)
					   (check-arg (list-ref f (- len 1)))))))))
			  
			  ((do)
			   (if (and (pair? (cdr arg))
				    (pair? (cddr arg)))
			       (let ((end+res (caddr arg)))
				 (check-arg (if (pair? (cdr end+res))
						(list-ref end+res (- (length end+res) 1))
						())))))
			  
			  ((case)
			   (for-each
			    (lambda (clause)
			      (if (and (pair? clause)
				       (pair? (cdr clause))
				       (not (eq? (cadr clause) '=>)))
				  (check-arg (list-ref clause (- (length clause) 1)))))
			    (cddr arg)))
			  
			  ((cond)
			   (for-each
			    (lambda (clause)
			      (if (pair? clause)
				  (if (pair? (cdr clause))
				      (if (not (eq? (cadr clause) '=>))
					  (check-arg (list-ref clause (- (length clause) 1))))
				      (check-cond-arg (car clause)))))
			    (cdr arg)))
			  
			  ((call/cc call-with-exit call-with-current-continuation)
			   ;; find func in body (as car of list), check its arg as return value
			   (when (and (pair? (cdr arg))
				      (pair? (cadr arg))
				      (eq? (caadr arg) 'lambda))
			     (let ((f (cdadr arg)))
			       (when (and (pair? f)
					  (pair? (car f))
					  (symbol? (caar f))
					  (null? (cdar f)))
				 (define c-walk 
				   (let ((rtn (caar f)))
				     (lambda (tree)
				       (if (pair? tree)
					   (if (eq? (car tree) rtn)
					       (check-arg (if (null? (cdr tree)) () (cadr tree)))
					       (begin
						 (c-walk (car tree))
						 (for-each (lambda (x) (if (pair? x) (c-walk x))) (cdr tree))))))))
				 (for-each c-walk (cdr f))))))
			  
			  ((values) 
			   (cond ((not (positive? (length arg))))
				 
				 ((null? (cdr arg)) ; #<unspecified>
				  (if (not (any-checker? checker #<unspecified>))
				      (report-arg-trouble caller form head arg-number checker arg 'unspecified?)))
				 
				 ((null? (cddr arg))
				  (check-arg (cadr arg)))
				 
				 (else
				  (for-each
				   (lambda (expr rest)
				     (check-arg expr)
				     (set! arg-number (+ arg-number 1))
				     (if (> arg-number max-arity) (done))
				     (if (list? checkers)
					 (if (null? (cdr checkers))
					     (done)
					     (set! checkers (cdr checkers)))))
				   (cdr arg) (cddr arg))
				  (check-arg (list-ref arg (- (length arg) 1))))))
			  
			  (else 
			   (let ((op (return-type (car arg) env)))
			     (let ((v (var-member (car arg) env)))
			       (if (and (var? v)
					(not (memq form (var-history v))))
				   (set! (var-history v) (cons form (var-history v)))))
			     
			     ;; checker is arg-type, op is expression type (can also be a pair)
			     (if (and (not (memq op '(#f #t values)))
				      (not (memq checker '(unused-parameter? unused-set-parameter?)))
				      (or (not (every-compatible? checker op))
					  (and (just-constants? arg env) ; try to eval the arg
					       (catch #t 
						 (lambda ()
						   (not (any-checker? checker (eval arg))))
						 (lambda ignore-catch-error-args
						   #f)))))
				 (report-arg-trouble caller form head arg-number checker arg op)))))))
		  
		  (if (list? checkers)
		      (if (null? (cdr checkers))
			  (done)
			  (set! checkers (cdr checkers)))
		      (if (memq checker '(unused-parameter? unused-set-parameter?))
			  (set! checker #t)))
		  (set! arg-number (+ arg-number 1))
		  (if (> arg-number max-arity) (done))))
	      (cdr form)))))))
    
    
    (define check-unordered-exprs
      (let ((changers (let ((h (make-hash-table)))
			(for-each (lambda (s) 
				    (hash-table-set! h s #t))
				  '(set!
				    read read-byte read-char read-line read-string 
				    write write-byte write-char write-string format display newline
				    reverse! set-cdr! sort! string-fill! vector-fill! fill! 
				    emergency-exit exit error throw))
			h)))
	(lambda (caller form vals env)
	  (define (report-trouble)
	    (lint-format* caller   ;  (let ((x (read-byte)) (y (read-byte))) (- x y))
			  (string-append "order of evaluation of " (object->string (car form)) "'s ")
			  (string-append (if (memq (car form) '(let letrec do)) "bindings" "arguments") " is unspecified, ")
			  (string-append "so " (truncated-list->string form) " is trouble")))
	  (let ((reads ())
		(writes ())
		(jumps ()))
	    (call-with-exit
	     (lambda (return)
	       (for-each (lambda (p)
			   (when (and (pair? p)
				      (not (var-member (car p) env))
				      (hash-table-ref changers (car p)))
			     (if (pair? jumps)
				 (return (report-trouble)))

			     (case (car p)
			       
			       ((read read-char read-line read-byte)
				(if (null? (cdr p))
				    (if (memq () reads)
					(return (report-trouble))
					(set! reads (cons () reads)))
				    (if (memq (cadr p) reads)
					(return (report-trouble))
					(set! reads (cons (cadr p) reads)))))
			       
			       ((read-string)
				(if (or (null? (cdr p))
					(null? (cddr p)))
				    (if (memq () reads)
					(return (report-trouble))
					(set! reads (cons () reads)))
				    (if (memq (caddr p) reads)
					(return (report-trouble))
					(set! reads (cons (caddr p) reads)))))
			       
			       ((display write write-char write-string write-byte)
				(if (null? (cddr p))
				    (if (memq () writes)
					(return (report-trouble))
					(set! writes (cons () writes)))
				    (if (memq (caddr p) writes)
					(return (report-trouble))
					(set! writes (cons (caddr p) writes)))))
			       
			       ((newline)
				(if (null? (cdr p))
				    (if (memq () writes)
					(return (report-trouble))
					(set! writes (cons () writes)))
				    (if (memq (cadr p) writes)
					(return (report-trouble))
					(set! writes (cons (cadr p) writes)))))
			       
			       ((format)
				(if (and (pair? (cdr p))
					 (not (string? (cadr p)))
					 (cadr p)) ; i.e. not #f
				    (if (memq (cadr p) writes)
					(return (report-trouble))
					(set! writes (cons (cadr p) writes)))))

			       ((fill! string-fill! vector-fill!  reverse! sort! set! set-cdr!)
				;; here there's trouble if cadr used anywhere -- but we need to check for shadowing
				(if (any? (lambda (np)
					    (and (not (eq? np p))
						 (tree-memq (cadr p) np)))
					  vals)
				    (return (report-trouble))))

			       ((throw error exit emergency-exit)
				(if (or (pair? reads)   ; jumps already checked above
					(pair? writes))
				    (return (report-trouble))
				    (set! jumps (cons p jumps)))))))
			 vals)))))))

    (define check-call 
      (let ((repeated-args-table (let ((h (make-hash-table)))
				   (for-each
				    (lambda (op)
				      (set! (h op) #t))
				    '(= / max min < > <= >= - quotient remainder modulo rationalize and or
					string=? string<=? string>=? string<? string>? string-ci=? string-ci<=? string-ci>=? string-ci<? string-ci>?
					char=? char<=? char>=? char<? char>? char-ci=? char-ci<=? char-ci>=? char-ci<? char-ci>?
					boolean=? symbol=?))
				   h))
	    (repeated-args-table-2 (let ((h (make-hash-table)))
				     (for-each
				      (lambda (op)
					(set! (h op) #t))
				      '(= max min < > <= >= and or
					  string=? string<=? string>=? string<? string>? string-ci=? string-ci<=? string-ci>=? string-ci<? string-ci>?
					  char=? char<=? char>=? char<? char>? char-ci=? char-ci<=? char-ci>=? char-ci<? char-ci>?
					  boolean=? symbol=?))
				     h)))
	(lambda (caller head form env)
	  (let ((data (var-member head env)))

	    (if (and (pair? (cdr form))
		     (pair? (cddr form))
		     (any-procedure? head env))
		(check-unordered-exprs caller form (cdr form) env))

	    (if (var? data)
		(let ((fdata (cdr data)))
		  ;; a local var
		  (when (symbol? (fdata 'ftype))
		    (let ((args (fdata 'arglist))
			  (ary (and (not (eq? (fdata 'decl) 'error))
				    (arity (fdata 'decl))))
			  (sig (var-signature data)))
		      (when (pair? ary)
			(let ((req (car ary))
			      (opt (cdr ary))
			      (pargs (if (pair? args) 
					 (proper-list args)
					 (if (symbol? args)
					     (list args)
					     ()))))
			  (let ((call-args (- (length form) 1)))
			    (if (< call-args req)
				(begin
				  (for-each (lambda (p)
					      (if (pair? p)
						  (let ((v (var-member (car p) env)))
						    (if (var? v)
							(let ((vals (let-ref (cdr v) 'values)))
							  (if (pair? vals)
							      (set! call-args (+ call-args -1 (cadr vals)))))))))
					    (cdr form))
				  (if (and (< call-args req)
					   (not (tree-memq 'values (cdr form))))
				      (lint-format "~A needs ~D argument~A: ~A" 
						   caller head 
						   req (if (> req 1) "s" "") 
						   (truncated-list->string form))))
				(if (> (- call-args (keywords (cdr form))) opt) ; multiple-values can make this worse, (values)=nothing doesn't apply here
				    (lint-format "~A has too many arguments: ~A" caller head (truncated-list->string form)))))

			  (unless (fdata 'allow-other-keys)
			    (let ((last-was-key #f)
				  (have-keys 0)
				  (warned #f)
				  (rest (if (and (pair? form) (pair? (cdr form))) (cddr form) ())))
			      (for-each
			       (lambda (arg)
				 (if (and (keyword? arg)
					  (not last-was-key))  ; keyarg might have key value
				     (begin
				       (set! have-keys (+ have-keys 1))
				       (if (not (member (keyword->symbol arg) pargs 
							(lambda (a b)
							  (eq? a (if (pair? b) (car b) b)))))
					   (lint-format "~A keyword argument ~A (in ~A) does not match any argument in ~S" caller 
							head arg (truncated-list->string form) pargs))
				       (if (memq arg rest)
					   (lint-format "~W is repeated in ~A" caller arg (cdr form)))
				       (set! last-was-key #t))
				     (begin
				       (when (and (positive? have-keys)
						  (not last-was-key)
						  (not warned))
					 (set! warned #t)
					 (lint-format "non-keyword argument ~A follows previous keyword~P" caller arg have-keys))
				       (set! last-was-key #f)))
				 (if (pair? rest)
				     (set! rest (cdr rest))))
			       (cdr form))))
			  
			  (check-args caller head form (if (pair? sig) (cdr sig) ()) env opt)
			  
			  ;; for a complete var-history, we could run through the args here even if no type info
			  ;; also if var passed to macro -- what to do?
			  
			  ;; look for problematic macro expansion
			  (when (memq (fdata 'ftype) '(define-macro define-macro* defmacro defmacro*))
			    
			    (unless (list? (fdata 'macro-ops))
			      (let ((syms (list () ())))
				(tree-symbol-walk ((if (memq (fdata 'ftype) '(define-macro define-macro*)) 
						       cddr cdddr)
						   (fdata 'initial-value))
						  syms)
				(varlet fdata 'macro-locals (car syms) 'macro-ops (cadr syms))))
			    
			    (when (or (pair? (fdata 'macro-locals))
				      (pair? (fdata 'macro-ops)))
			      (let ((bad-locals ())
				    (bad-quoted-locals ()))
				(for-each
				 (lambda (local)
				   (if (tree-unquoted-member local (cdr form))
				       (set! bad-locals (cons local bad-locals))))
				 (fdata 'macro-locals))
				(when (null? bad-locals)
				  (for-each
				   (lambda (local)
				     (if (tree-member local (cdr form))
					 (set! bad-quoted-locals (cons local bad-quoted-locals))))
				   (fdata 'macro-locals)))
				(let ((bad-ops ()))
				  (for-each
				   (lambda (op)
				     (let ((curf (var-member op env))
					   (oldf (var-member op (fdata 'env))))
				       (if (and (not (eq? curf oldf))
						(or (pair? (fdata 'env))
						    (defined? op (rootlet))))
					   (set! bad-ops (cons op bad-ops)))))
				   (fdata 'macro-ops))
				  
				  (when (or (pair? bad-locals)
					    (pair? bad-quoted-locals) 
					    ;; (define-macro (mac8 b) `(let ((a 12)) (+ (symbol->value ,b) a)))
					    ;; (let ((a 1)) (mac8 'a))
					    ;; far-fetched!
					    (pair? bad-ops))
				    (lint-format "possible problematic macro expansion:~%  ~A ~A collide with subsequently defined ~A~A~A" 
						 caller 
						 (truncated-list->string form)
						 (if (or (pair? bad-locals)
							 (pair? bad-ops))
						     "may"
						     "could conceivably")
						 (if (pair? bad-locals)
						     (format #f "~{'~A~^, ~}" bad-locals)
						     (if (pair? bad-quoted-locals)
							 (format #f "~{'~A~^, ~}" bad-quoted-locals)
							 ""))
						 (if (and (pair? bad-locals) (pair? bad-ops)) ", " "")
						 (if (pair? bad-ops)
						     (format #f "~{~A~^, ~}" bad-ops)
						     "")))))))
			  )))))
		;; not local var
		(when (symbol? head)
		  (let ((head-value (symbol->value head *e*))) ; head might be "arity"!
		    (when (or (procedure? head-value)
			      (macro? head-value))
		      ;; check arg number
		      (let ((ary (arity head-value)))
			(let ((args (- (length form) 1))
			      (min-arity (car ary))
			      (max-arity (cdr ary)))
			  (if (< args min-arity)
			      (lint-format "~A needs ~A~D argument~A: ~A" 
					   caller head 
					   (if (= min-arity max-arity) "" "at least ")
					   min-arity
					   (if (> min-arity 1) "s" "") 
					   (truncated-list->string form))
			      (if (and (not (procedure-setter head-value))
				       (> (- args (keywords (cdr form))) max-arity))
				  (lint-format "~A has too many arguments: ~A" caller head (truncated-list->string form))))
			  
			  (when (and (procedure? head-value)
				     (pair? (cdr form))) ; there are args (the not-enough-args case is checked above)
			    (if (zero? max-arity)
				(lint-format "too many arguments: ~A" caller (truncated-list->string form))    
				(begin
				  
				  (for-each (lambda (arg)
					      (if (pair? arg)
						  (if (negative? (length arg))
						      (lint-format "missing quote? ~A in ~A" caller arg form)
						      (if (eq? (car arg) 'unquote)
							  (lint-format "stray comma? ~A in ~A" caller arg form)))))
					    (cdr form))
				  
				  ;; if keywords, check that they are acceptable
				  ;;    this only applies to lambda*'s that have been previously loaded (lint doesn't create them)
				  (let ((source (procedure-source head-value)))
				    (if (and (pair? source)
					     (eq? (car source) 'lambda*))
					(let ((decls (cadr source)))
					  (if (not (memq :allow-other-keys decls))
					      (for-each
					       (lambda (arg)
						 (if (and (keyword? arg)
							  (not (eq? arg :rest))
							  (not (member arg decls 
								       (lambda (a b) 
									 (eq? (keyword->symbol a) (if (pair? b) (car b) b))))))
						     (lint-format "~A keyword argument ~A (in ~A) does not match any argument in ~S" caller 
								  head arg (truncated-list->string form) decls)))
					       (cdr form))))))
				  
				  ;; we've already checked for head in the current env above
				  (if (and (or (memq head '(eq? eqv?))
					       (and (= (length form) 3)
						    (hash-table-ref repeated-args-table head)))
					   (repeated-member? (cdr form) env))
				      (lint-format "this looks odd: ~A"
						   caller
						   ;; sigh (= a a) could be used to check for non-finite numbers, I suppose,
						   ;;   and (/ 0 0) might be deliberate (as in gmp)
						   ;;   also (min (random x) (random x)) is not pointless
						   (truncated-list->string form))
				      (if (and (hash-table-ref repeated-args-table-2 head)
					       (repeated-member? (cdr form) env))
					  (lint-format "it looks odd to have repeated arguments in ~A" caller (truncated-list->string form))))
				  
				  (when (memq head '(eq? eqv?))
				    (define (repeated-member-with-not? lst env)
				      (and (pair? lst)
					   (let ((this-repeats (and (not (and (pair? (car lst))
									      (side-effect? (car lst) env)))
								    (or (member (list 'not (car lst)) (cdr lst))
									(and (pair? (car lst))
									     (eq? (caar lst) 'not)
									     (= (length (car lst)) 2)
									     (member (cadar lst) (cdr lst)))))))
					     (or this-repeats
						 (repeated-member-with-not? (cdr lst) env)))))
				    (if (repeated-member-with-not? (cdr form) env)
					(lint-format "this looks odd: ~A" caller (truncated-list->string form))))
				  
				  ;; now try to check arg types 
				  (let ((arg-data (cond ((procedure-signature (symbol->value head *e*)) => cdr) (else #f))))
				    (if (pair? arg-data)
					(check-args caller head form arg-data env max-arity))
				    ))))))))))))))
    
    (define (indirect-set? vname func arg1)
      (case func
	((set-car! set-cdr! vector-set! list-set! string-set!) 
	 (eq? arg1 vname))
	((set!) 
	 (and (pair? arg1) 
	      (eq? (car arg1) vname)))
	(else #f)))

    (define (env-difference name e1 e2 lst)
      (if (or (null? e1)
	      (null? e2)
	      (eq? (car e1) (car e2)))
	  (reverse lst)
	  (env-difference name (cdr e1) e2 
			  (if (eq? name (var-name (car e1)))
			      lst
			      (cons (car e1) lst)))))

    (define report-usage 
      (let ((unwrap-cxr (hash-table '(caar car) '(cadr cdr) '(cddr cdr) '(cdar car)
				    '(caaar caar car) '(caadr cadr cdr) '(caddr cddr cdr) '(cdddr cddr cdr)
				    '(cdaar caar car) '(cddar cdar car) '(cadar cadr car) '(cdadr cadr cdr)
				    '(cadddr cdddr cddr cdr) '(cddddr cdddr cddr cdr) '(caaaar caaar caar car) '(caaadr caadr cadr cdr)
				    '(caadar cadar cdar car) '(caaddr caddr cddr cdr) '(cadaar cdaar caar car) '(cadadr cdadr cadr cdr)
				    '(caddar cddar cdar car) '(cdaaar caaar caar car) '(cdaadr caadr cadr cdr) '(cdadar cadar cdar car)
				    '(cdaddr caddr cddr cdr) '(cddaar cdaar caar car) '(cddadr cdadr cadr cdr) '(cdddar cddar cdar car))))

	(lambda (caller head vars env)
	  ;; report unused or set-but-unreferenced variables, then look at the overall history
	  
	  (define (all-types-agree v)
	    (let ((base-type (->lint-type (var-initial-value v)))
		  (vname (var-name v)))
	      (and (every? (lambda (p)
			     (or (not (and (pair? p)
					   (eq? (car p) 'set!)
					   (eq? vname (cadr p))))
				 (let ((nt (->lint-type (caddr p))))
				   (or (subsumes? base-type nt)
				       (and (subsumes? nt base-type)
					    (set! base-type nt))
				       (and (memq nt '(pair? null? proper-list?))
					    (memq base-type '(pair? null? proper-list?))
					    (set! base-type 'list?))))))
			   (var-history v))
		   base-type)))
	  
	  (when (and (not (eq? head 'begin)) ; begin can redefine = set a variable
		     (pair? vars)
		     (proper-list? vars))
	    (do ((cur vars (cdr cur))
		 (rst (cdr vars) (cdr rst)))
		((null? rst))
	      (let ((vn (var-name (car cur))))
		(if (not (memq vn '(:lambda :dilambda)))
		    (let ((repeat (var-member vn rst)))
		      (when repeat
			(let ((type (if (eq? (var-definer repeat) 'parameter) 'parameter 'variable)))
			  (if (eq? (var-definer (car cur)) 'define)
			      (lint-format "~A ~A ~A is redefined ~A" caller head type vn 
					   (if (equal? head "")
					       (if (not (tree-memq vn (var-initial-value (car cur))))
						   "at the top level."
						   (format #f "at the top level. Perhaps use set! instead: ~A"
							   (truncated-list->string `(set! ,vn ,(var-initial-value (car cur))))))
					       (format #f "in the ~A body.  Perhaps use set! instead: ~A"
						       head (truncated-list->string `(set! ,vn ,(var-initial-value (car cur)))))))
			      (lint-format "~A ~A ~A is declared twice" caller head type vn)))))))))
	  
	  (let ((old-line-number line-number))
	    
	    (for-each 
	     (lambda (local-var)
	       (let ((vname (var-name local-var))
		     (otype (if (eq? (var-definer local-var) 'parameter) 'parameter 'variable)))
		 
		 ;; do all refs to an unset var go through the same function (at some level)
		 (when (and (zero? (var-set local-var))
			    (> (var-ref local-var) 1)
			    (not (eq? otype 'parameter)))
		   (let ((hist (var-history local-var)))
		     (when (pair? hist)
		       (let ((outer-form (cond ((var-member :let env) => var-initial-value) (else #f))))
			 ;; if outer-form is #f, local-var is probably a top-level var
			 (when (and (pair? outer-form)
				    (not (and (memq (car outer-form) '(let let*))  ; not a named-let parameter
					      (symbol? (cadr outer-form)))))
			   (let ((first (car hist)))                               ; all but the initial binding have to match this
			     (when (pair? first)
			       (let ((op (car first)))
				 (when (and (symbol? op)
					    (not (eq? op 'unquote))
					    (not (hash-table-ref makers op))
					    (not (eq? vname op))                   ; not a function (this kind if repetition is handled elsewhere)
					    (pair? (cdr hist))
					    (pair? (cddr hist))
					    (pair? (cdr first))
					    (not (side-effect? first env))
					    (every? (lambda (a)
						      (or (eq? a vname)
							  (code-constant? a)))
						    (cdr first))
					    (or (code-constant? (var-initial-value local-var))
						(= (tree-count1 vname first 0) 1))
					    (every? (lambda (a) 
						      (and (pair? a)
							   (or (equal? first a)
							       (and (eq? (hash-table-ref reversibles (car first)) (car a))
								    (equal? (cdr first) (reverse (cdr a))))
							       (set! op (match-cxr op (car a))))))
						    (copy (cdr hist) (make-list (- (length hist) 2)))))
				   (let* ((new-op (or op (car first)))
					  (set-target (let walker ((tree outer-form)) ; check for new-op dilambda as target of set!
							(and (pair? tree)
							     (or (and (eq? (car tree) 'set!)
								      (pair? (cdr tree))
								      (pair? (cadr tree))
								      (eq? (caadr tree) new-op))
								 (walker (car tree))
								 (walker (cdr tree)))))))
				     (unless set-target
				       (lint-format* caller 
						     (symbol->string vname)
						     " is not set, and is always accessed via "
						     (object->string `(,new-op ,@(cdr first)))
						     " so its binding could probably be "
						     ;;        "probably" here because the accesses could have hidden protective assumptions
						     ;;          i.e. full accessor is not valid at point of let binding
						     (object->string `(,vname (,new-op ,@(tree-subst (var-initial-value local-var) vname (cdr first)))))
						     " in "
						     (truncated-list->string outer-form)))))))))))))
		 
		 ;; translate to dilambda fixing arg if necessary and mention generic set!
		 (let ((init (var-initial-value local-var)))
		   (when (and (pair? init)
			      (eq? (car init) 'define)
			      (pair? (cadr init)))
		     (let* ((vstr (symbol->string vname))
			    (len (length vstr)))
		       (when (> len 4)
			 (let ((setv #f)
			       (newv #f))
			   (if (string=? (substring vstr 0 4) "get-")
			       (let ((sv (symbol "set-" (substring vstr 4))))
				 (set! setv (or (var-member sv vars)
						(var-member sv env)))
				 (set! newv (string->symbol (substring vstr 4))))
			       (if (string=? (substring vstr (- len 4)) "-ref")
				   (let ((sv (symbol (substring vstr 0 (- len 4)) "-set!")))
				     (set! setv (or (var-member sv vars)
						    (var-member sv env)))
				     (set! newv (string->symbol (substring vstr 0 (- len 4)))))
				   (let ((pos (string-position "-get-" vstr)))
				     (when pos ; this doesn't happen very often, others: Get-, -ref-, -set!- are very rare
				       (let ((sv (let ((s (copy vstr))) (set! (s (+ pos 1)) #\s) (string->symbol s))))
					 (set! setv (or (var-member sv vars)
							(var-member sv env)))
					 (set! newv (symbol (substring vstr 0 pos) 
							    (substring vstr (+ pos 4))))))))) ; +4 to include #\-
			   (when (and setv 
				      (not (var-member newv vars))
				      (not (var-member newv env)))
			     (let ((getter init)
				   (setter (var-initial-value setv)))
			       (when (and (pair? setter)
					  (eq? (car setter) 'define)
					  (pair? (cadr setter)))
				 (let ((getargs (cdadr getter))
				       (setargs (cdadr setter)))
				   (unless (null? setargs)
				     (if (or (eq? newv getargs)
					     (and (pair? getargs)
						  (memq newv getargs)))
					 (let ((unique (find-unique-name getter newv)))
					   (set! getter (tree-subst unique newv getter))
					   (set! getargs (cdadr getter))))
				     (if (or (eq? newv setargs)
					     (and (pair? setargs)
						  (memq newv setargs)))
					 (let ((unique (find-unique-name setter newv)))
					   (set! setter (tree-subst unique newv setter))
					   (set! setargs (cdadr setter))))
				     (let ((getdots (if (null? getargs) "" " ..."))
					   (setdots (if (or (not (pair? setargs)) (null? (cdr setargs))) "" " ..."))
					   (setvalue (and (proper-list? setargs)
							  (list-ref setargs (- (length setargs) 1)))))
				       (if setvalue
					   (format outport "~NC~A: perhaps use dilambda and generalized set! for ~A and ~A:~%~
                                                  ~NCreplace (~A~A) with (~A~A) and (~A~A ~A) with (set! (~A~A) ~A)~%~
                                                  ~NC~A~%"
						   lint-left-margin #\space
						   caller
						   vname (var-name setv)
						   (+ lint-left-margin 4) #\space
						   vname getdots newv getdots
						   (var-name setv) setdots setvalue
						   newv setdots setvalue
						   (+ lint-left-margin 4) #\space
						   (lint-pp `(define ,newv (dilambda 
									    (lambda ,getargs ,@(cddr getter)) 
									    (lambda ,setargs ,@(cddr setter))))))))))))))))))
		 ;; bad variable names
		 (cond ((hash-table-ref syntaces vname)
			(lint-format "~A ~A named ~A is asking for trouble" caller head otype vname))
		       
		       ((eq? vname 'l)
			(lint-format "\"l\" is a really bad variable name" caller))
		       
		       ((and *report-built-in-functions-used-as-variables*
			     (hash-table-ref built-in-functions vname))
			(lint-format "~A ~A named ~A is asking for trouble" caller 
				     (if (and (pair? (var-scope local-var))
					      (null? (cdr (var-scope local-var)))
					      (symbol? (car (var-scope local-var))))
					 (car (var-scope local-var))
					 head)
				     otype vname))
		       
		       (else (check-for-bad-variable-name caller vname)))
		 
		 (unless (memq vname '(:lambda :dilambda))
		   (if (and (eq? otype 'variable)
			    (or *report-unused-top-level-functions*
				(not (eq? caller top-level:))))
		       (let ((scope (var-scope local-var))) ; might be #<undefined>?
			 (if (pair? scope) (set! scope (remove vname scope)))
			 
			 (when (and (pair? scope)
				    (null? (cdr scope))
				    (symbol? (car scope))
				    (not (var-member (car scope) (let search ((e env))
								   (if (null? e)
								       env
								       (if (eq? (caar e) vname)
									   e
									   (search (cdr e))))))))
			   (format outport "~NC~A~A is ~A only in ~A~%" 
				   lint-left-margin #\space 
				   (if (eq? caller top-level:)
				       "top-level: "
				       "")
				   vname 
				   (if (memq (var-ftype local-var) '(define lambda define* lambda*)) "called" "used")
				   (car scope)))))
		   
		   (if (and (eq? (var-ftype local-var) 'define-expansion)
			    (not (eq? caller top-level:)))
		       (format outport "~NCdefine-expansion for ~A is not at the top-level, so it is ignored~%" 
			       lint-left-margin #\space
			       vname))
		   
		   (when (and *report-function-stuff*
			      (memq (var-ftype local-var) '(define lambda define* lambda*))
			      (pair? (caddr (var-initial-value local-var))))
		     (let ((cur (hash-table-ref equable-closures (caaddr (var-initial-value local-var)))))
		       (if (pair? cur)
			   (hash-table-set! equable-closures (caaddr (var-initial-value local-var)) (remove local-var cur)))))
		   
		   ;; redundant vars are hard to find -- tons of false positives
		   
		   (if (zero? (var-ref local-var))
		       (when (and (or (not (equal? head ""))
				      *report-unused-top-level-functions*)
				  (or *report-unused-parameters*
				      (not (eq? otype 'parameter))))
			 (if (positive? (var-set local-var))
			     (let ((sets (map (lambda (call)
						(if (and (pair? call)
							 (not (eq? (var-definer local-var) 'do))
							 (eq? (car call) 'set!)
							 (eq? (cadr call) vname))
						    call
						    (values)))
					      (var-history local-var))))
			       (if (pair? sets)
				   (if (null? (cdr sets))
				       (lint-format "~A set, but not used: ~A" caller 
						    vname (truncated-list->string (car sets)))
				       (lint-format "~A set, but not used: ~{~S~^ ~}" caller 
						    vname sets))
				   (lint-format "~A set, but not used: ~A from ~A" caller 
						vname (truncated-list->string (var-initial-value local-var)) (var-definer local-var))))
			     
			     ;; not ref'd or set
			     (if (not (memq vname '(documentation signature iterator? defanimal)))
				 (let ((val (if (pair? (var-history local-var)) (car (var-history local-var)) (var-initial-value local-var)))
				       (def (var-definer local-var)))
				   (let-temporarily ((line-number (if (eq? caller top-level:) -1 line-number)))
				     ;; eval confuses this message (eval '(+ x 1)), no other use of x [perhaps check :let initial-value = outer-form]
				     ;;    so does let-ref syntax: (apply (*e* 'g1)...) will miss this reference to g1
				     (if (symbol? def)
					 (if (eq? otype 'parameter)
					     (lint-format "~A not used" caller vname)
					     (lint-format* caller 
							   (string-append (object->string vname) " not used, initially: ")
							   (string-append (truncated-list->string val) " from " (symbol->string def))))
					 (lint-format* caller 
						       (string-append (object->string vname) " not used, value: ")
						       (truncated-list->string val))))))))
		       ;; not zero var-ref
		       (let ((arg-type #f))
			 
			 (when (and (not (memq (var-definer local-var) '(parameter named-let named-let*)))
				    (pair? (var-history local-var))
				    (or (zero? (var-set local-var))
					(set! arg-type (all-types-agree local-var))))
			   (let ((vtype (or arg-type                ; this can't be #f unless no sets so despite appearances there's no contention here
					    (eq? caller top-level:) ; might be a global var where init value is largely irrelevant
					    (->lint-type (var-initial-value local-var))))
				 (lit? (code-constant? (var-initial-value local-var))))
			     
			     (do ((clause (var-history local-var) (cdr clause)))
				 ((null? (cdr clause)))             ; ignore the initial value which depends on a different env
			       (let ((call (car clause)))
				 (if (pair? call) (set! line-number (pair-line-number call)))
				 
				 (when (pair? call)
				   (let ((func (car call))
					 (call-arg1 (and (pair? (cdr call)) (cadr call))))
				     
				     ;; check for assignments into constants
				     (if (and lit?
					      (indirect-set? vname func call-arg1))
					 (lint-format "~A's value, ~A, is a literal constant, so this set! is trouble: ~A" caller 
						      vname (var-initial-value local-var) (truncated-list->string call)))
				     
				     (when (symbol? vtype)
				       (when (and (not (eq? caller top-level:))
						  (not (memq vtype '(boolean? #t values)))
						  (memq func '(if when unless)) ; look for (if x ...) where x is never #f, this happens a dozen or so times
						  (or (eq? (cadr call) vname)
						      (and (pair? (cadr call))
							   (eq? (caadr call) 'not)
							   (eq? (cadadr call) vname))))
					 (lint-format "~A is never #f, so ~A" caller 
						      vname 
						      (lists->string 
						       call
						       (if (eq? vname (cadr call))
							   (case func
							     ((if) (caddr call))
							     ((when) (if (pair? (cdddr call)) `(begin ,@(cddr call)) (caddr call)))
							     ((unless) #<unspecified>))
							   (case func
							     ((if) (if (pair? (cdddr call)) (cadddr call)))
							     ((when) #<unspecified>)
							     ((unless) (if (pair? (cdddr call)) `(begin ,@(cddr call)) (caddr call))))))))
				       
				       ;; check for incorrect types in function calls
				       (unless (memq vtype '(boolean? null?)) ; null? here avoids problems with macros that call set!
					 (let ((p (memq vname (cdr call))))                    
					   (when (pair? p)
					     (let ((sig (arg-signature func env))
						   (pos (- (length call) (length p))))
					       (when (and (pair? sig)
							  (< pos (length sig)))
						 (let ((desired-type (list-ref sig pos)))
						   (if (not (compatible? vtype desired-type))
						       (lint-format "~A is ~A, but ~A in ~A wants ~A" caller
								    vname (prettify-checker-unq vtype)
								    func (truncated-list->string call) 
								    (prettify-checker desired-type))))))))
					 
					 (let ((suggest made-suggestion))
					   ;; check for pointless vtype checks
					   (when (and (hash-table-ref bools func)
						      (not (eq? vname func)))
					     
					     (when (or (eq? vtype func)
						       (and (compatible? vtype func)
							    (not (subsumes? vtype func))))
					       (lint-format "~A is ~A, so ~A is #t" caller vname (prettify-checker-unq vtype) call))
					     
					     (unless (compatible? vtype func)
					       (lint-format "~A is ~A, so ~A is #f" caller vname (prettify-checker-unq vtype) call)))
					   
					   (case func
					     ;; need a way to mark exported variables so they won't be checked in this process
					     ;; case can happen here, but it never seems to trigger a type error
					     ((eq? eqv? equal?)
					      ;; (and (pair? x) (eq? x #\a)) etc
					      (when (or (and (code-constant? call-arg1)
							     (not (compatible? vtype (->lint-type call-arg1))))
							(and (code-constant? (caddr call))
							     (not (compatible? vtype (->lint-type (caddr call))))))
						(lint-format "~A is ~A, so ~A is #f" caller vname (prettify-checker-unq vtype) call)))
					     
					     ((and or)
					      (when (let amidst? ((lst call))
						      (and (pair? lst)
							   (pair? (cdr lst))
							   (or (eq? (car lst) vname)
							       (amidst? (cdr lst)))))   ; don't clobber possible trailing vname (returned by expression)
						(lint-format "~A is ~A, so ~A~%" caller       ; (let ((x 1)) (and x (< x 1))) -> (< x 1)
							     vname (prettify-checker-unq vtype)
							     (lists->string call 
									    (simplify-boolean (remove vname call) () () vars)))))
					     ((not)
					      (if (eq? vname (cadr call))
						  (lint-format "~A is ~A, so ~A" caller
							       vname (prettify-checker-unq vtype)
							       (lists->string call #f))))
					     
					     ((/) (if (and (number? (var-initial-value local-var))
							   (zero? (var-initial-value local-var))
							   (zero? (var-set local-var))
							   (memq vname (cddr call)))
						      (lint-format "~A is ~A, so ~A is an error" caller
								   vname (var-initial-value local-var)
								   call))))
					   
					   ;; the usual eqx confusion
					   (when (and (= suggest made-suggestion)
						      (memq vtype '(char? number? integer? real? float? rational? complex?)))
					     (if (memq func '(eq? equal?))
						 (lint-format "~A is ~A, so ~A ~A be eqv? in ~A" caller 
							      vname (prettify-checker-unq vtype) func
							      (if (eq? func 'eq?) "should" "could")
							      call))
					     ;; check other boolean exprs
					     (when (and (zero? (var-set local-var))
							(number? (var-initial-value local-var))
							(eq? vname call-arg1)
							(null? (cddr call))
							(hash-table-ref booleans func))
					       (let ((val (catch #t 
							    (lambda ()
							      ((symbol->value func (rootlet)) (var-initial-value local-var)))
							    (lambda args 
							      'error))))
						 (if (boolean? val)
						     (lint-format "~A is ~A, so ~A is ~A" caller vname (var-initial-value local-var) call val))))))
					 
					 ;; implicit index checks -- these are easily fooled by macros
					 (when (and (memq vtype '(vector? float-vector? int-vector? string? list? byte-vector?))
						    (pair? (cdr call)))
					   (when (eq? func vname)
					     (let ((init (var-initial-value local-var)))
					       (if (not (compatible? 'integer? (->lint-type call-arg1)))
						   (lint-format "~A is ~A, but the index ~A is ~A" caller
								vname (prettify-checker-unq vtype)
								call-arg1 (prettify-checker (->lint-type call-arg1))))
					       
					       (if (integer? call-arg1)
						   (if (negative? call-arg1)
						       (lint-format "~A's index ~A is negative" caller vname call-arg1)
						       (if (zero? (var-set local-var))
							   (let ((lim (cond ((code-constant? init)
									     (length init))
									    
									    ((memq (car init) '(vector float-vector int-vector string list byte-vector))
									     (- (length init) 1))
									    
									    (else
									     (and (pair? (cdr init))
										  (integer? (cadr init))
										  (memq (car init) '(make-vector make-float-vector make-int-vector 
														 make-string make-list make-byte-vector))
										  (cadr init))))))
							     (if (and (real? lim)
								      (>= call-arg1 lim))
								 (lint-format "~A has length ~A, but index is ~A" caller vname lim call-arg1))))))))
					   
					   (when (eq? func 'implicit-set)
					     ;; ref is already checked in other history entries
					     (let ((ref-type (case vtype
							       ((float-vector?) 'real?) ; not 'float? because ints are ok here
							       ((int-vector? byte-vector?) 'integer)
							       ((string?) 'char?)
							       (else #f))))
					       (if ref-type
						   (let ((val-type (->lint-type (caddr call))))
						     (if (not (compatible? val-type ref-type))
							 (lint-format "~A wants ~A, but the value in ~A is ~A" caller
								      vname (prettify-checker-unq ref-type)
								      `(set! ,@(cdr call)) 
								      (prettify-checker val-type)))))
					       ))))))
				   ))) ; do loop through clauses
			     
			     ;; check for duplicated calls involving local-var
			     (when (and (> (var-ref local-var) 8)
					(zero? (var-set local-var))
					(eq? (var-ftype local-var) #<undefined>))
			       (let ((h (make-hash-table)))
				 (for-each (lambda (call)
					     (when (and (pair? call)
							(not (eq? (car call) vname)) ; ignore functions for now
							(not (side-effect? call env)))
					       (hash-table-set! h call (+ 1 (or (hash-table-ref h call) 0)))
					       (cond ((hash-table-ref unwrap-cxr (car call))
						      => (lambda (lst)
							   (for-each (lambda (c)
								       (hash-table-set! h (cons c (cdr call)) (+ 1 (or (hash-table-ref h (cons c (cdr call))) 0))))
								     lst))))))
					   (var-history local-var))
				 (let ((repeats ()))
				   (for-each (lambda (call)
					       (if (and (> (cdr call) (max 3 (/ 20 (tree-leaves (car call))))) ; was 5
							(not (memq (caar call) '(make-vector make-float-vector)))
							(or (null? (cddar call))
							    (every? (lambda (p)
								      (or (not (symbol? p))
									  (eq? p vname)))
								    (cdar call))))
						   (set! repeats (cons (string-append (truncated-list->string (car call)) " occurs ")
								       (cons (string-append (object->string (cdr call)) " times"
											    (if (pair? repeats) ", " ""))
									     repeats)))))
					     h)
				   (if (pair? repeats)
				       (apply lint-format* 
					      caller
					      (string-append (object->string vname) " is not set, but ")
					      repeats)))))
			     
			     ;; check for function parameters whose values never change and are not just symbols
			     (when (and (> (var-ref local-var) 3)
					(zero? (var-set local-var))
					(memq (var-ftype local-var) '(define lambda))
					(pair? (var-arglist local-var))
					(let loop ((calls (var-history local-var)))         ; if func passed as arg, ignore it
					  (or (null? calls)
					      (null? (cdr calls))
					      (and (pair? (car calls))
						   (not (memq (var-name local-var) (cdar calls)))
						   (loop (cdr calls))))))
			       (let ((pars (map list (proper-list (var-arglist local-var)))))
				 (do ((clauses (var-history local-var) (cdr clauses)))
				     ((null? (cdr clauses))) ; ignore the initial value
				   (if (and (pair? (car clauses))
					    (eq? (caar clauses) (var-name local-var)))
				       (for-each (lambda (arg par)                           ; collect all arguments for each parameter
						   (if (not (member arg (cdr par)))          ; we haven't seen this argument yet, so
						       (set-cdr! par (cons arg (cdr par))))) ;   add it to the list for this parameter
						 (cdar clauses)
						 pars)))
				 (for-each (lambda (p)
					     (if (and (pair? (cdr p))
						      (null? (cddr p))                       ; so all calls, this parameter has the same value
						      (not (symbol? (cadr p))))
						 (lint-format "~A's '~A parameter is always ~S (~D calls)" caller
							      (var-name local-var) (car p) (cadr p) (var-ref local-var))))
					   pars)))
			     )))) ; end (if zero var-ref)
		   
		   ;; vars with multiple incompatible ascertainable types don't happen much and obvious type errors are extremely rare
		   )))
	     vars)
	    (set! line-number old-line-number)))))


    ;; ----------------------------------------
    ;; preloading built-in definitions, and looking for them here found less than a dozen (list-ref, list-tail, and boolean?)

    (define (code-equal? p1 p2 matches e1 e2)

      (define (match-vars r1 r2 mat)
	(and (pair? r1)
	     (pair? r2)
	     (pair? (cdr r1))
	     (pair? (cdr r2))
	     ((if (and (pair? (cadr r1))
		       (pair? (cadr r2))
		       (memq (caadr r1) '(let let* letrec letrec* do lambda lambda* 
					  define define-constant define-macro define-bacro define-expansion
					  define* define-macro* define-bacro*)))
		 code-equal? structures-equal?)
	      (cadr r1) (cadr r2) mat e1 e2)
	     (cons (car r1) (car r2))))
	
      (let ((f1 (car p1))
	    (f2 (car p2)))
	(and (eq? f1 f2)
	     (let ((rest1 (cdr p1))
		   (rest2 (cdr p2)))
	       (and (pair? rest1)
		    (pair? rest2)
		    (call-with-exit
		     (lambda (return)
		       (case f1
			 ((let)
			  (let ((name ()))
			    (if (symbol? (car rest1))                      ; named let -- match funcs too
				(if (symbol? (car rest2))
				    (begin
				      (set! name (list (cons (car rest1) (car rest2))))
				      (set! rest1 (cdr rest1))
				      (set! rest2 (cdr rest2)))
				    (return #f))
				(if (symbol? (car rest2))
				    (return #f)))
			    (and (= (length (car rest1)) (length (car rest2)))
				 (let ((new-matches (append (map (lambda (var1 var2)
								   (or (match-vars var1 var2 matches)
								       (return #f)))
								 (car rest1)
								 (car rest2))
							    name              ; append will splice out nil
							    matches)))
				   (structures-equal? (cdr rest1) (cdr rest2) ; refs in values are to outer matches
						      new-matches e1 e2)))))
			 ((let*)                                           ; refs move with the vars 
			  (and (= (length (car rest1)) (length (car rest2)))
			       (let ((new-matches matches))
				 (for-each (lambda (var1 var2)
					     (cond ((match-vars var1 var2 new-matches) =>
						    (lambda (v)
						      (set! new-matches (cons v new-matches))))
						   (else (return #f))))
					   (car rest1)
					   (car rest2))
				 (structures-equal? (cdr rest1) (cdr rest2) new-matches e1 e2))))
			 
			 ((do)                                             ; matches at init are outer, but at step are inner
			  (and (= (length (car rest1)) (length (car rest2)))
			       (let ((new-matches matches))
				 (for-each (lambda (var1 var2)
					     (cond ((match-vars var1 var2 matches) =>
						    (lambda (v)
						      (set! new-matches (cons v new-matches))))
						   (else (return #f))))
					   (car rest1)
					   (car rest2))
				 (for-each (lambda (var1 var2)
					     (unless (structures-equal? (cddr var1) (cddr var2) new-matches e1 e2)
					       (return #f)))
					   (car rest1)
					   (car rest2))
				 (structures-equal? (cdr rest1) (cdr rest2) new-matches e1 e2))))
			 
			 ((letrec letrec*)                                ; ??? refs are local I think
			  (and (= (length (car rest1)) (length (car rest2)))
			       (let ((new-matches (append (map (lambda (var1 var2)
								 (cons (car var1) (car var2)))
							       (car rest1)
							       (car rest2))
							  matches)))
				 (for-each (lambda (var1 var2)
					     (unless (structures-equal? (cadr var1) (cadr var2) new-matches e1 e2)
					       (return #f)))
					   (car rest1)
					   (car rest2))
				 (structures-equal? (cdr rest1) (cdr rest2) new-matches e1 e2))))
			 
			 ((lambda) 
			  (if (symbol? (car rest1))
			      (and (symbol? (car rest2))
				   (structures-equal? (cdr rest1) (cdr rest2)
						      (cons (cons (car rest1) (car rest2)) matches) e1 e2))
			      (and (eqv? (length (car rest1)) (length (car rest2))) ; (car rest2) might be a symbol, dotted lists ok here
				   (let ((new-matches (append (map cons (proper-list (car rest1)) (proper-list (car rest2)))
							      matches)))
				     (structures-equal? (cdr rest1) (cdr rest2) new-matches e1 e2)))))

			 ((define define-constant define-macro define-bacro define-expansion)
			  (if (symbol? (car rest1))
			      (and (symbol? (car rest2))
				   (let ((new-matches (cons (cons (car rest1) (car rest2)) matches)))
				     (and (structures-equal? (cdr rest1) (cdr rest2) new-matches e1 e2)
					  new-matches)))
			      (and (eqv? (length (car rest1)) (length (car rest2))) ; (car rest2) might be a symbol, dotted lists ok here
				   (let ((new-matches (append (map cons (proper-list (car rest1)) (proper-list (car rest2)))
							      matches)))
				     (structures-equal? (cdr rest1) (cdr rest2) new-matches e1 e2))
				   (cons (cons (caar rest1) (caar rest2)) matches))))
			 ;; for define we add the new name to matches before walking the body (shadow is immediate),
			 ;;   but then the new name is added to matches and returned (see below)
			 
			 ((lambda*)
			  (if (symbol? (car rest1))
			      (and (symbol? (car rest2))
				   (structures-equal? (cdr rest1) (cdr rest2)
						      (cons (cons (car rest1) (car rest2)) matches)
						      e1 e2))
			      (and (eqv? (length (car rest1)) (length (car rest2))) ; (car rest2) might be a symbol, dotted lists ok here
				   (let ((new-matches (map (lambda (a b)
							     (if (or (pair? a)  ; if default, both must have the same value
								     (pair? b))
								 (if (not (and (pair? a)
									       (pair? b)
									       (equal? (cadr a) (cadr b))))
								     (return #f)
								     (cons (car a) (car b)))
								 (cons a b)))
							   (proper-list (car rest1)) (proper-list (car rest2)))))
				     (structures-equal? (cdr rest1) (cdr rest2)
							(append new-matches matches)
							e1 e2)))))
			 
			 ((define* define-macro* define-bacro*)
			  (if (symbol? (car rest1))
			      (and (symbol? (car rest2))
				   (let ((new-matches (cons (cons (car rest1) (car rest2)) matches)))
				     (and (structures-equal? (cdr rest1) (cdr rest2) new-matches e1 e2)
					  new-matches)))
			      (and (eqv? (length (car rest1)) (length (car rest2))) ; (car rest2) might be a symbol, dotted lists ok here
				   (let ((new-matches (map (lambda (a b)
							     (if (or (pair? a)  ; if default, both must have the same value
								     (pair? b))
								 (if (not (and (pair? a)
									       (pair? b)
									       (equal? (cadr a) (cadr b))))
								     (return #f)
								     (cons (car a) (car b)))
								 (cons a b)))
							   (proper-list (car rest1)) (proper-list (car rest2)))))
				     (structures-equal? (cdr rest1) (cdr rest2) 
							(append new-matches new-matches)
							e1 e2))
				   (cons (cons (caar rest1) (caar rest2)) matches))))
			 
			 (else #f))))))))) ; can't happen I hope

    (define (structures-equal? p1 p2 matches e1 e2)
      (if (pair? p1)
	  (and (pair? p2)
	       (if (eq? (car p1) 'quote)
		   (and (eq? (car p2) 'quote)
			(equal? (cdr p1) (cdr p2)))
		   (and (if (not (and (pair? (car p1))
				      (pair? (car p2))))
			    (structures-equal? (car p1) (car p2) matches e1 e2)
			    (case (caar p1)
			      ((let let* letrec letrec* do lambda lambda*)
			       (code-equal? (car p1) (car p2) matches e1 e2))
			      
			      ((define define-constant define-macro define-bacro define-expansion define* define-macro* define-bacro*)
			       (let ((mat (code-equal? (car p1) (car p2) matches e1 e2)))
				 (and (pair? mat)
				      (set! matches mat))))
			      
			      ;; this ignores possible reversible equivalence (i.e. (< x 0) is the same as (> 0 x)
			      ;;   (structures-equal? (car p1) (car p2) matches e1 e2)))
			      
			      ;; check for reversible equivalence
			      ;;   half-humorous problem: infinite loop here switching back and forth!
			      ;;   so I guess we have to check cdar by hand
			      ;; we could also check for not+notable here, but lint will complain
			      ;;   about that elsewhere, causing this check to be ignored.
			      (else (or (structures-equal? (car p1) (car p2) matches e1 e2)
					(and (eq? (hash-table-ref reversibles (caar p1)) (caar p2))
					     (not (any? (lambda (p) (side-effect? p e1)) (cdar p1))) ; (+ (oscil g) (oscil g x)) is not reversible!
					     (do ((a (cdar p1) (cdr a))
						  (b (reverse (cdar p2)) (cdr b)))
						 ((or (null? a)
						      (null? b)
						      (not (structures-equal? a b matches e1 e2)))
						  (and (null? a)
						       (null? b)))))))))
			(structures-equal? (cdr p1) (cdr p2) matches e1 e2))))
	  (let ((match (assq p1 matches)))
	    (if match
		(or (and (eq? (cdr match) :unset)
			 (set-cdr! match p2))
		    (equal? (cdr match) p2))
		(if (symbol? p1)
		    (and (eq? p1 p2)
			 (or (eq? e1 e2)
			     (eq? (assq p1 e1) (assq p2 e2))))
		    (equal? p1 p2))))))


    ;; code-equal? and structures-equal? called in function-match and each other

    (define (function-match caller form env)

      (define func-min-cutoff 6)
      (define func-max-cutoff 120)

      (define (proper-list* lst)
	;; return lst as a proper list (might have defaults, keywords etc)
	(if (or (not (pair? lst))
		(eq? (car lst) :allow-other-keys))
	    ()
	    (if (eq? (car lst) :rest)
		(cdr lst)
		(cons ((if (pair? (car lst)) caar car) lst)
		      (if (pair? (cdr lst)) 
			  (proper-list* (cdr lst)) 
			  (if (null? (cdr lst)) 
			      () 
			      (list (cdr lst))))))))
      
      (let ((leaves (tree-leaves form)))

	(when (<= func-min-cutoff leaves func-max-cutoff)
	  (let ((new-form (if (pair? (car form)) form (list form)))
		(name-args #f)
		(name-args-len :unset)
		(e2 ()))
	    
	    (let ((v (var-member caller env)))
	      (when (and (var? v)
			 (memq (var-ftype v) '(define lambda define* lambda*))
			 (or (eq? form (cddr (var-initial-value v)))    ; only check args if this is the complete body
			     (and (null? (cdddr (var-initial-value v))) 
				  (eq? form (caddr (var-initial-value v))))))
		(set! e2 (var-env v))
		(if (symbol? (var-arglist v))
		    (begin
		      (set! name-args-len #f)
		      (set! name-args (list (var-arglist v))))
		    (begin
		      (set! name-args-len (length (var-arglist v)))
		      (set! name-args (map (lambda (arg)
					     (if (symbol? arg)
						 arg
						 (values)))
					   (proper-list* (var-arglist v))))))))
	    
	    (let ((find-code-match 
		   (let ((e1 ())
			 (cutoff (max func-min-cutoff (- leaves 12))))
		     (lambda (v)
		       (and (not (memq (var-name v) '(:lambda :dilambda)))
			    (memq (var-ftype v) '(define lambda define* lambda*))
			    (not (eq? caller (var-name v)))
			    (let ((body (cddr (var-initial-value v)))
				  (args (var-arglist v)))
			      (set! e1 (var-env v))
			      
			      (let ((args-len (length args)))
				(when (or (eq? name-args-len :unset)
					  (equal? args-len name-args-len)
					  (and (integer? args-len)
					       (integer? name-args-len)
					       (not (negative? (* args-len name-args-len)))))
				  
				  (unless (var-leaves v)
				    (set! (var-leaves v) (tree-leaves body))
				    (set! (var-match-list v) (if (symbol? args)
								 (list (cons args :unset))
								 (map (lambda (arg)
									(if (symbol? arg)
									    (cons arg :unset)
									    (values)))
								      (proper-list* args)))))
				  
				  ;; var-leaves is size of func (v) body
				  ;; leaves is size of form which we want to match with func
				  ;; func-min-cutoff avoids millions of uninteresting matches
				  
				  (and (<= cutoff (var-leaves v) leaves)
				       (let ((match-list (do ((p (var-match-list v) (cdr p))) 
							     ((null? p) 
							      (var-match-list v))
							   (set-cdr! (car p) :unset))))
					 (and (structures-equal? body new-form
								 (cons (cons (var-name v) caller) match-list) e1 e2)
					      ;; if the functions are recursive, we also need those names matched, hence the extra entry
					      ;;   but we treat match-list below as just the args, so add the func names at the call,
					      ;;   but this can be fooled if we're playing games with eq? basically -- the function
					      ;;   names should only match if used as functions.
					      
					      (not (member :unset match-list (lambda (a b) (eq? (cdr b) :unset))))
					      (let ((new-args (map cdr match-list)))
						(if (and (equal? new-args name-args)
							 (equal? args-len name-args-len))
						    (lint-format "~A could be ~A" caller caller `(define ,caller ,(var-name v)))
						    (lint-format "perhaps ~A" caller (lists->string form `(,(var-name v) ,@new-args))))
						#t))))))))))))
		
	      (do ((vs (or (hash-table-ref equable-closures (caar new-form)) ()) (cdr vs)))
		  ;; instead of hashing on car as above, hash on composite of cars+base statements
		  ((or (null? vs)
		       (find-code-match (car vs))))))))))
    

    (define (find-call sym body)
      (call-with-exit
       (lambda (return)
	 (let tree-call ((tree body))
	   (if (and (pair? tree)
		    (not (eq? (car tree) 'quote)))
	       (begin
		 (if (eq? (car tree) sym)
		     (return tree))
		 (if (memq (car tree) '(let let* letrec letrec* do lambda lambda* define))
		     (return #f)) ; possible shadowing -- not worth the infinite effort to corroborate
		 (if (pair? (car tree))
		     (tree-call (car tree)))
		 (if (pair? (cdr tree))
		     (do ((p (cdr tree) (cdr p)))
			 ((not (pair? p)) #f)
		       (tree-call (car p))))))))))


    (define (check-returns caller f env) ; f is not the last form in the body
      (if (not (or (side-effect? f env)
		   (eq? '=> f)))
	  (lint-format "this could be omitted: ~A" caller (truncated-list->string f))
	  (when (pair? f)
	    (case (car f)
	      ((if)
	       (when (and (pair? (cdr f))
			  (pair? (cddr f)))
		 (let ((true (caddr f))
		       (false (if (pair? (cdddr f)) (cadddr f) 'no-false)))
		   (let ((true-ok (side-effect? true env))
			 (false-ok (or (eq? false 'no-false)
				       (side-effect? false env))))
		     (if true-ok
			 (if (pair? true)
			     (check-returns caller true env))
			 (lint-format "this branch is pointless~A: ~A in ~A" caller
				      (local-line-number true)
				      (truncated-list->string true)
				      (truncated-list->string f)))
		     (if false-ok
			 (if (pair? false)
			     (check-returns caller false env))
			 (lint-format "this branch is pointless~A: ~A in ~A" caller
				      (local-line-number false)
				      (truncated-list->string false)
				      (truncated-list->string f)))))))
	      ((cond case)
	       ;; here all but last result exprs are already checked
	       ;;   redundant begin can confuse this, but presumably we'll complain about that elsewhere
	       ;;   also even in mid-body, if else clause has a side-effect, an earlier otherwise pointless clause might be avoiding that
	       (let ((has-else (let ((last-clause (list-ref f (- (length f) 1))))
				 (and (pair? last-clause)
				      (memq (car last-clause) '(else #t))
				      (any? (lambda (c)
					      (side-effect? c env))
					    (cdr last-clause))))))
		 (for-each (lambda (c)
			     (if (and (pair? c)
				      (pair? (cdr c))
				      (not (memq '=> (cdr c))))
				 (let ((last-expr (list-ref c (- (length c) 1))))
				   (cond ((side-effect? last-expr env)
					  (if (pair? last-expr)
					      (check-returns caller last-expr env)))

					 (has-else
					  (if (or (pair? (cddr c))
						  (eq? (car f) 'cond))
					      (lint-format "this ~A clause's result could be omitted" caller 
							   (truncated-list->string c))
					      (if (not (memq last-expr '(#f #t #<unspecified>))) ; it's not already obvious
						  (lint-format "this ~A clause's result could be simply #f" caller
							       (truncated-list->string c)))))
					 ((and (eq? (car f) 'case)
					       (or (eq? last-expr (cadr c))
						   (not (any? (lambda (p) (side-effect? p env)) (cdr c)))))
					  (lint-format "this case clause can be omitted: ~A" caller
						       (truncated-list->string c)))

					 (else (lint-format "this is pointless: ~A in ~A" caller
							    (truncated-list->string last-expr)
							    (truncated-list->string c)))))))
			   ((if (eq? (car f) 'cond) cdr cddr) f))))
	      
	      ((let let*)
	       (if (and (pair? (cdr f))
			(not (symbol? (cadr f)))
			(pair? (cddr f)))
		   (let ((last-expr (list-ref f (- (length f) 1))))
		     (if (side-effect? last-expr env)
			 (if (pair? last-expr)
			     (check-returns caller last-expr env))		 
			 (lint-format "this is pointless~A: ~A in ~A" caller
				      (local-line-number last-expr)
				      (truncated-list->string last-expr)
				      (truncated-list->string f))))))

	      ;; perhaps use truncated-lists->string here??
	      ((and)
	       (let ((len (length f)))
		 (case len
		   ((1) (lint-format "this ~A is pointless" caller f))
		   ((2) (lint-format "perhaps ~A" caller (lists->string f (cadr f))))
		   ((3) (lint-format "perhaps ~A" caller (lists->string f `(if ,(cadr f) ,(caddr f))))) ; (begin (and x (display y)) (log z)) -> (if x (display y))
		   (else (lint-format "perhaps ~A" caller (lists->string f `(if ,(cadr f) (and ,@(cddr f)))))))))

	      ((or)
	       (let ((len (length f)))
		 (case len
		   ((1) (lint-format "this ~A is pointless" caller f))
		   ((2) (lint-format "perhaps ~A" caller (lists->string f (cadr f))))
		   ((3) (lint-format "perhaps ~A" caller (lists->string f `(if (not ,(cadr f)) ,(caddr f)))))
		   (else (lint-format "perhaps ~A" caller (lists->string f `(if (not ,(cadr f)) (or ,@(cddr f)))))))))

	      ((not)
	       (lint-format "this ~A is pointless" caller f))

	      ((letrec letrec* with-let unless when begin with-baffle)
	       (if (and (pair? (cdr f))
			(pair? (cddr f)))
		   (let ((last-expr (list-ref f (- (length f) 1))))
		     (if (side-effect? last-expr env)
			 (if (pair? last-expr)
			     (check-returns caller last-expr env))
			 ;; (begin (if x (begin (display x) z)) z)
			 (lint-format "this is pointless~A: ~A in ~A" caller
				      (local-line-number last-expr)
				      (truncated-list->string last-expr)
				      (truncated-list->string f))))))
	      ((do)
	       (let ((returned (if (and (pair? (cdr f))
					(pair? (cddr f)))
				   (let ((end+res (caddr f)))
				     (if (pair? (cdr end+res))
					 (list-ref end+res (- (length end+res) 1)))))))
		 (if (or (eq? returned #<unspecified>)
			 (and (pair? returned)
			      (side-effect? returned env)))
		     (if (pair? returned)
			 (check-returns caller returned env))
		     ;; (begin (do ((i 0 (+ i 1))) ((= i 10) i) (display i)) x)
		     (lint-format "~A: result ~A~A is not used" caller 
				  (truncated-list->string f) 
				  (truncated-list->string returned)
				  (local-line-number returned)))))
	      ((call-with-exit)
	       (if (and (pair? (cdr f))
			(pair? (cadr f))
			(eq? (caadr f) 'lambda)
			(pair? (cdadr f))
			(pair? (cadadr f)))
		   (let ((return (car (cadadr f))))
		     (let walk ((tree (cddadr f)))
		       (if (pair? tree)
			   (if (eq? (car tree) return)
			       (if (and (pair? (cdr tree))
					(or (not (boolean? (cadr tree)))
					    (pair? (cddr tree))))
				   ;; (begin (call-with-exit (lambda (quit) (if (< x 0) (quit (+ x 1))) (display x))) (+ x 2))
				   (lint-format "th~A call-with-exit return value~A will be ignored: ~A" caller
						(if (pair? (cddr tree)) 
						    (values "ese" "s")
						    (values "is" ""))
						tree))
			       (for-each walk tree)))))))

	      ((map)
	       (if (pair? (cdr f))          ;  (begin (map g123 x) x)
		   (lint-format "map could be for-each: ~A" caller (truncated-list->string `(for-each ,@(cdr f))))))

	      ((reverse!)
	       (if (pair? (cdr f))          ;  (let ((x (list 23 1 3))) (reverse! x) x)
		   (lint-format "~A might leave ~A in an undefined state; perhaps ~A" caller (car f) (cadr f)
				`(set! ,(cadr f) ,f))))

	      ((format)
	       (if (and (pair? (cdr f))
			(eq? (cadr f) #t))  ;  (let () (format #t "~A" x) x)
		   (lint-format "perhaps use () with format since the string value is discarded:~%    ~A" 
				caller `(format () ,@(cddr f)))))))))

    (define lint-current-form #f)
    (define lint-mid-form #f)

    (define (escape? form env)
      (and (pair? form)
	   (let ((v (var-member (car form) env)))
	     (if (var? v)
		 (memq (var-definer v) '(call/cc call-with-current-continuation call-with-exit))
		 (memq (car form) '(error throw))))))


    (define (lint-walk-body caller head body env)
      (when (pair? body)
	(when (and (pair? (car body))
		   (pair? (cdar body)))
	  (when (and (not (eq? last-rewritten-internal-define (car body))) ; we already rewrote this
		     (pair? (cdr body))                ; define->named let, but this is only ok in a "closed" situation, not (begin (define...)) for example
		     (pair? (cadr body))
		     (memq (caar body) '(define define*))
		     (pair? (cadar body)))
	    (let ((fname (caadar body))
		  (fargs (cdadar body))
		  (fbody (cddar body)))
	      (when (and (symbol? fname)
			 (proper-list? fargs)
			 (= (tree-count1 fname (cdr body) 0) 1)
			 (not (any? keyword? fargs)))
		(let ((call (find-call fname (cdr body))))
		  (when (pair? call)
		    (let ((new-args (if (eq? (caar body) 'define)
					(map list fargs (cdr call))
					(let loop ((pars fargs)
						   (vals (cdr call))
						   (args ()))
					  (if (null? pars)
					      (reverse args)
					      (loop (cdr pars)
						    (if (pair? vals)
							(values (cdr vals) 
								(cons (list ((if (pair? (car pars)) caar car) pars) (car vals)) args))
							(values () 
								(cons (if (pair? (car pars)) (car pars) (list (car pars) #f)) args))))))))
			  (new-let (if (eq? (caar body) 'define) 'let 'let*)))
		      (if (and (pair? fbody)
			       (pair? (cdr fbody))
			       (string? (car fbody)))
			  (set! fbody (cdr fbody)))
		      ;; (... (define* (f1 a b) (+ a b)) (f1 :c 1)) -> (... (let ((a :c) (b 1)) (+ a b)))
		      (lint-format "perhaps ~A" caller
				   (lists->string `(... ,@body)
						  (if (= (tree-count2 fname body 0) 2)
						      (if (null? fargs)
							  (if (null? (cdr fbody))
							      `(... ,@(tree-subst (car fbody) call (cdr body)))
							      `(... ,@(tree-subst `(let () ,@fbody) call (cdr body))))
							  `(... ,@(tree-subst `(let ,new-args ,@fbody) call (cdr body))))
						      `(... ,@(tree-subst `(,new-let ,fname ,new-args ,@fbody) call (cdr body))))))))))))
	  
	  ;; look for non-function defines at the start of the body and use let(*) instead
	  ;;   we're in a closed body here, so the define can't propagate backwards

	  (let ((first-expr (car body)))
	    ;; another case: f(args) (let(...)set! arg < no let>)
	    (when (and (eq? (car first-expr) 'define)
		       (symbol? (cadr first-expr))
		       (pair? (cddr first-expr))
		       ;;(not (tree-car-member (cadr first-expr) (caddr first-expr)))
		       ;;(not (tree-set-car-member '(lambda lambda*) (caddr first-expr)))
		       (not (and (pair? (caddr first-expr))
				 (memq (caaddr first-expr) '(lambda lambda*)))))
	      ;; this still is not ideal -- we need to omit let+lambda as well
	      (do ((names ())
		   (letx 'let)
		   (vars&vals ())
		   (p body (cdr p)))
		  ((not (and (pair? p)
			     (let ((expr (car p)))
			       (and (pair? expr)
				    (eq? (car expr) 'define)
				    (symbol? (cadr expr))
				    (pair? (cddr expr))
				    ;;(not (tree-set-car-member '(lambda lambda*) (caddr expr)))))
				    (not (and (pair? (caddr expr))
					      (memq (caaddr expr) '(lambda lambda*))))))))
		   ;; (... (define x 3) 32) -> (... (let ((x 3)) ...))
		   (lint-format "perhaps ~A" caller
				(lists->string `(... ,@body)
					       `(... (,letx ,(reverse vars&vals)
							    ...)))))
		;; define acts like letrec(*), not let -- reference to name in lambda body is current name
		(let ((expr (cdar p)))
		  (set! vars&vals (cons (if (< (tree-leaves (cdr expr)) 12)
					    expr 
					    (list (car expr) '...))
					vars&vals))
		  (if (tree-set-member names (cdr expr))
		      (set! letx 'let*))
		  (set! names (cons (car expr) names)))))))
	
	(let ((len (length body)))
	  (when (> len 2)                           ; ... (define (x...)...) (x ...) -> (let (...) ...) or named let -- this happens a lot!
	    (let ((n-1 (list-ref body (- len 2)))   ; or (define (x ...)...) (some expr calling x once) -> named let etc
		  (n (list-ref body (- len 1))))
	      (when (and (pair? n-1)
			 (eq? (car n-1) 'define)
			 (pair? (cadr n-1))
			 (symbol? (caadr n-1))
			 (proper-list? (cdadr n-1))
			 (pair? n)
			 (or (and (eq? (car n) (caadr n-1))
				  (eqv? (length (cdadr n-1)) (length (cdr n)))) ; not values -> let!
			     (and (< (tree-leaves n-1) 12)
				  (tree-car-member (caadr n-1) (cdr n))         ; skip car -- see preceding
				  (= (tree-count1 (caadr n-1) n 0) 1))))
		(let ((outer-form (cond ((var-member :let env) => var-initial-value) (else #f)))
		      (new-var (caadr n-1)))
		  (when (and (pair? outer-form)
			     (not (let walker ((tree outer-form)) ; check even the enclosing env -- define in do body back ref'd in stepper for example
				    (or (eq? new-var tree)
					(and (pair? tree)
					     (not (eq? n tree))
					     (not (eq? n-1 tree))
					     (not (eq? (car tree) 'quote))
					     (or (walker (car tree))
						 (walker (cdr tree))))))))
		    (let ((named (if (tree-memq new-var (cddr n-1)) (list new-var) ())))
		      (if (eq? (car n) (caadr n-1))
			  (lint-format "perhaps change ~A to a ~Alet: ~A" caller new-var (if (pair? named) "named " "")
				       (lists->string outer-form `(... (let ,@named ,(map list (cdadr n-1) (cdr n)) ...))))
			  (let ((call (find-call new-var n)))
			    (when (and (pair? call)
				       (eqv? (length (cdadr n-1)) (length (cdr call))))
			      (let ((new-call `(let ,@named ,(map list (cdadr n-1) (cdr call)) ,@(cddr n-1))))
				(lint-format "perhaps embed ~A: ~A" caller new-var
					     (lists->string outer-form `(... ,(tree-subst new-call call n))))))))))))))

	  ;; this comment has strayed?
	  ;; needs to check outer let also -- and maybe complain? [outer = form: we're already closed?]
	  ;; bounds of closable context might be dependent on body length
	  ;; (let ((outer-form (cond ((var-member :let env) => var-initial-value) (else #f)))
	  ;;  if used just once, move to that point in the expr+1? -- need to point it out somehow?

	  (let ((suggest made-suggestion))

	  (when (and (> len 2)
		     (not (tree-memq 'curlet (list-ref body (- len 1)))))
	    (do ((q body (cdr q))
		 (k 0 (+ k 1)))
		((null? q))
	      (let ((expr (car q)))
		(when (and (pair? expr)
			   (eq? (car expr) 'define)
			   (pair? (cdr expr))
			   (pair? (cddr expr))
			   (null? (cdddr expr)))
		  (let ((name (and (symbol? (cadr expr)) (cadr expr))))
		    (when name
		      (do ((last-ref k)
			   (p (cdr q) (cdr p))
			   (i (+ k 1) (+ i 1)))
			  ((null? p)
			   (if (and (< k last-ref (+ k 2)) 
				    (pair? (list-ref body (+ k 1))))
			       (let ((end-dots (if (< last-ref (- len 1)) '(...) ()))
				     (letx (if (tree-member name (cddr expr)) 'letrec 'let))
				     (use-expr (list-ref body (+ k 1)))
				     (seen-earlier (or (var-member name env)
						       (do ((s body (cdr s)))
							   ((or (eq? s q)
								(and (pair? (car s))
								     (tree-memq name (car s))))
							    (not (eq? s q)))))))
				 (cond (seen-earlier)
				       
				       ((not (eq? (car use-expr) 'define))
					(let-temporarily ((target-line-length 120))
					  ;; (... (define f14 (lambda (x y) (if (positive? x) (+ x y) y))) (+ (f11 1 2) (f14 1 2))) ->
					  ;;    (... (let ((f14 (lambda (x y) (if (positive? x) (+ x y) y)))) (+ (f11 1 2) (f14 1 2))))
					  (lint-format "the scope of ~A could be reduced: ~A" caller name
						       (truncated-lists->string `(... ,expr ,use-expr ,@end-dots)
										`(... (,letx ((,name ,(caddr expr))) 
											     ,use-expr)
										      ,@end-dots)))))
				       ((eq? (cadr use-expr) name)
					;; (let () (display 33) (define x 2) (define x (+ x y)) (display 43)) ->
					;;    (... (set! x (+ x y)) ...)
					(lint-format "use set! to redefine ~A: ~A" caller name
						     (lists->string `(... ,use-expr ,@end-dots)
								    `(... (set! ,name ,(caddr use-expr)) ,@end-dots))))
				       ((pair? (cadr use-expr))
					(if (symbol? (caadr use-expr))
					    (let-temporarily ((target-line-length 120))
					      ;; (let () (display 32) (define x 2) (define (f101 y) (+ x y)) (display 41) (f101 2)) ->
					      ;;    (... (define f101 (let ((x 2)) (lambda (y) (+ x y)))) ...)
					      (lint-format "perhaps move ~A into ~A's closure: ~A" caller name (caadr use-expr)
							   (truncated-lists->string `(... ,expr ,use-expr ,@end-dots)
										    `(... (define ,(caadr use-expr)
											    (,letx ((,name ,(caddr expr)))
												   (lambda ,(cdadr use-expr)
												     ,@(cddr use-expr))))
											  ,@end-dots))))))
				       ((and (symbol? (cadr use-expr))
					     (pair? (cddr use-expr)))
					(let-temporarily ((target-line-length 120))
					  (if (and (pair? (caddr use-expr))
						   (eq? (caaddr use-expr) 'lambda))
					      ;; (let () (display 34) (define x 2) (define f101 (lambda (y) (+ x y))) (display 41) (f101 2))
					      ;;    (... (define f101 (let ((x 2)) (lambda (y) (+ x y)))) ...)
					      (lint-format "perhaps move ~A into ~A's closure: ~A" caller name (cadr use-expr)
							   (truncated-lists->string `(... ,expr ,use-expr ,@end-dots)
										    `(... (define ,(cadr use-expr)
											    (,letx ((,name ,(caddr expr)))
												   ,(caddr use-expr)))
											  ,@end-dots)))
					      ;; (... (define lib (r file)) (define exports (caddr lib)) ...) ->
					      ;;    (... (define exports (let ((lib (r file))) (caddr lib))) ...)
					      (lint-format "the scope of ~A could be reduced: ~A" caller name
							   (truncated-lists->string `(... ,expr ,use-expr ,@end-dots)
										    `(... (define ,(cadr use-expr)
											    (,letx ((,name ,(caddr expr)))
												   ,(caddr use-expr)))
											  ,@end-dots))))))))
			       (when (and (> len 3)
					  (< k last-ref (+ k 3))  ; larger cases happen very rarely -- 3 or 4 altogether
					  (pair? (list-ref body (+ k 1)))
					  (pair? (list-ref body (+ k 2))))
				 (let ((end-dots (if (< last-ref (- len 1)) '(...) ()))
				       (letx (if (tree-member name (cddr expr)) 'letrec 'let))
				       (seen-earlier (or (var-member name env)
							 (do ((s body (cdr s)))
							     ((or (eq? s q)
								  (and (pair? (car s))
								       (tree-memq name (car s))))
							      (not (eq? s q)))))))
				   (unless seen-earlier
				     (let ((use-expr1 (list-ref body (+ k 1)))
					   (use-expr2 (list-ref body (+ k 2))))
				       (if (not (or (tree-set-member '(define lambda) use-expr1)
						    (tree-set-member '(define lambda) use-expr2)))
					   ;; (... (define f101 (lambda (y) (+ x y))) (display 41) (f101 2)) ->
					   ;;    (... (let ((f101 (lambda (y) (+ x y)))) (display 41) (f101 2)))
					   (lint-format "the scope of ~A could be reduced: ~A" caller name
							(let-temporarily ((target-line-length 120))
							  (truncated-lists->string `(... ,expr ,use-expr1 ,use-expr2 ,@end-dots)
										   `(... (,letx ((,name ,(caddr expr))) 
											    ,use-expr1
											    ,use-expr2)
											 ,@end-dots)))))))))))
			  (when (tree-memq name (car p))
			    (set! last-ref i)))))))))

	  (when (= suggest made-suggestion)
	    ;; look for define+binding-expr at end and combine
	    (do ((prev-f #f)
		 (fs body (cdr fs)))
		((not (pair? fs)))
	      (let ((f (car fs)))
		;; define can come after the use, and in an open body can be equivalent to set!:
		;;   (let () (if x (begin (define y 12) (do ((i 0 (+ i 1))) ((= i y)) (f i))) (define y 21)) y)
		;;   (let () (define (f x) (+ y x)) (if z (define y 12) (define y 1)) (f 12))
		;; so we can't do this check in walk-open-body
		;;
		;; define + do -- if cadr prev-f not used in do inits, fold into do, else use let
		;;   the let case is semi-redundant (it's already reported elsewhere)
		(when (and (pair? prev-f)
			   (pair? f)
			   (eq? (car prev-f) 'define)
			   (symbol? (cadr prev-f))
			   (not (hash-table-ref other-identifiers (cadr prev-f))) ; (cadr prev-f) already ref'd, so it's a member of env
			   (or (null? (cdr fs))
			       (not (tree-memq (cadr prev-f) (cdr fs)))))
		  (if (eq? (car f) 'do)
		      ;; (... (define z (f x)) (do ((i z (+ i 1))) ((= i 3)) (display (+ z i))) ...) -> (do ((i (f x) (+ i 1))) ((= i 3)) (display (+ z i)))
		      (lint-format "perhaps ~A" caller
				   (lists->string `(... ,prev-f ,f ...)
						  (if (any? (lambda (p)
							      (tree-memq (cadr prev-f) (cadr p)))
							    (cadr f))
						      (if (and (eq? (cadr prev-f) (cadr (caadr f)))
							       (null? (cdadr f)))
							  `(do ((,(caaadr f) ,(caddr prev-f) ,(caddr (caadr f)))) ,@(cddr f))
							  `(let (,(cdr prev-f)) ,f))
						      `(do (,(cdr prev-f)
							    ,@(cadr f))
							   ,@(cddr f)))))
		      ;; just changing define -> let seems officious, though it does reduce (cadr prev-f)'s scope
		      (if (and (or (and (eq? (car f) 'let)
					(not (tree-memq (cadr prev-f) (cadr f))))
				   (eq? (car f) 'let*))
			       (not (symbol? (cadr f))))
			  (lint-format "perhaps ~A" caller
				       (lists->string 
					`(... ,prev-f ,f ,@(if (null? (cdr fs)) () '(...)))
					`(... (,(car f) (,(cdr prev-f) ,@(cadr f)) ...) ,@(if (null? (cdr fs)) () '(...))))))))
		(set! prev-f f)))))))

      ;; definer as last in body is rare outside let-syntax, and tricky -- only one clear optimizable case found
      (lint-walk-open-body caller head body env))


    (define (lint-walk-open-body caller head body env)
      ;; walk a body (a list of forms, the value of the last of which might be returned)
      
      (if (not (proper-list? body))
	  (lint-format "stray dot? ~A" caller (truncated-list->string body))
	  
	  (let ((prev-f #f)
		(old-current-form lint-current-form)
		(old-mid-form lint-mid-form)
		(prev-len 0)
		(f-len 0)
		(repeats 0)
		(start-repeats body)
		(repeat-arg 0)
		(dpy-f #f)
		(dpy-start #f)
		(rewrote-already #f)
		(len (length body)))
	    (if (eq? head 'do) (set! len (+ len 1))) ; last form in do body is not returned

	    (when (and (pair? body)
		       *report-function-stuff*
		       (not (null? (cdr body))))
	      (function-match caller body env))

	    (do ((fs body (cdr fs))
		 (ctr 0 (+ ctr 1)))
		((not (pair? fs)))
	      (let* ((f (car fs))
		     (f-func (and (pair? f) (car f))))

		(when (and (pair? f)
			   (pair? (cdr f)))
		  (if (eq? f-func 'define)
		      (let ((vname (if (symbol? (cadr f)) 
				       (cadr f) 
				       (and (pair? (cadr f))
					    (symbol? (caadr f))
					    (caadr f)))))
			;; if already in env, check shadowing request
			(if (and *report-shadowed-variables*
				 (var-member vname env))
			    ;; (let ((f33 33)) (define f33 4) (g f33 1))
			    (lint-format "~A variable ~A in ~S shadows an earlier declaration" caller head vname f))))
		  ;; mid-body defines happen by the million, so resistance is futile

		  ;; -------- repeated if/when etc --------
		  (when (and (pair? prev-f) ; (if A ...) (if A ...) -> (when A ...) or equivalents
			     (memq (car prev-f) '(if when unless))
			     (memq f-func '(if when unless))
			     (pair? (cdr prev-f))
			     (pair? (cddr f))  ; possible broken if statement
			     (pair? (cddr prev-f)))
		    
		    (define (tree-change-member set tree)
		      (and (pair? tree)
			   (not (eq? (car tree) 'quote))
			   (or (and (eq? (car tree) 'set!)
				    (memq (cadr tree) set))
			       (tree-change-member set (car tree))
			       (tree-change-member set (cdr tree)))))
		    
		    (let ((test1 (cadr prev-f))
			  (test2 (cadr f)))
		      (let ((equal-tests  ; test1 = test2
			     (lambda ()
			       ;; (... (if (and A B) (f C)) (if (and B A) (g E) (h F)) ...) -> (... (if (and A B) (begin (f C) (g E)) (begin (h F))) ...)
			       (lint-format "perhaps ~A" caller
					    (lists->string
					     `(... ,prev-f ,f ...)
					     (if (eq? f-func 'if)
						 (if (and (null? (cdddr prev-f))
							  (null? (cdddr f)))
						     ;; if (null (cdr fs)) we have to make sure the returned value is not changed by our rewrite
						     ;;   but when/unless return their last value in s7 (or #<unspecified>), so I think this is ok
						     (if (and (pair? test1)
							      (eq? (car test1) 'not))
							 `(... (unless ,(cadr test1)
								 ,@(unbegin (caddr prev-f))
								 ,@(unbegin (caddr f))) ...)
							 `(... (when ,test1
								 ,@(unbegin (caddr prev-f))
								 ,@(unbegin (caddr f))) ...))
						     `(... (if ,test1
							       (begin
								 ,@(unbegin (caddr prev-f))
								 ,@(unbegin (caddr f)))
							       (begin
								 ,@(if (pair? (cdddr prev-f)) (unbegin (cadddr prev-f)) ())
								 ,@(if (pair? (cdddr f)) (unbegin (cadddr f)) ())))
							   ...))
						 `(,f-func ,test1 ; f-func = when|unless
						   ,@(cddr prev-f)
						   ,@(cddr f)))))))
			    (test1-in-test2 
			     (lambda ()
			       (if (null? (cddr test2))
				   (set! test2 (cadr test2)))
			       ;;     (... (if A (f B)) (when (and A C) (g D) (h E)) ...) -> (... (when A (f B) (when C (g D) (h E))) ...)
			       (lint-format "perhaps ~A" caller
					    (lists->string `(... ,prev-f ,f ...)
							   (if (or (null? (cdddr prev-f))
								   (eq? (car prev-f) 'when)) ; so prev-f is when or 1-arm if (as is f)
							       `(... (when ,test1
								       ,@(cddr prev-f)
								       (when ,test2
									 ,@(cddr f))) 
								     ,@(if (null? (cdr fs)) () '(...)))
							       ;; prev-f is 2-arm if and f is when or 1-arm if (the other case is too ugly)
							       `(... (if ,test1
									 (begin
									   ,(caddr prev-f)
									   (when ,test2
									     ,@(cddr f)))
									 ,@(cdddr prev-f)) ...))))))
			    
			    (test2-in-test1 
			     (lambda ()
			       (if (null? (cddr test1))
				   (set! test1 (cadr test1)))
			       ;; (... (if (and A B) (f C)) (if A (g E)) ...) -> (... (when A (when B (f C)) (g E)))
			       (lint-format "perhaps ~A" caller
					    (lists->string `(... ,prev-f ,f ...)
							   (if (or (null? (cdddr f))
								   (eq? f-func 'when)) ; so f is when or 1-arm if (as is prev-f)
							       `(... (when ,test2
								       (when ,test1
									 ,@(cddr prev-f))
								       ,@(cddr f))
								     ,@(if (null? (cdr fs)) () '(...)))
							       ;; f is 2-arm if and prev-f is when or 1-arm if
							       `(... (if ,test2
									 (begin
									   (when ,test1
									     ,@(cddr prev-f))
									   ,(caddr f))
									 ,(cadddr f))
								     ,@(if (null? (cdr fs)) () '(...)))))))))
			(cond ((equal? test1 test2)
			       (if (and (eq? f-func (car prev-f))
					(not (side-effect? test1 env))
					(not (tree-change-member (gather-symbols test1) (cdr prev-f))))
				   (equal-tests)))
			    
			      ((or (eq? f-func 'unless)
				   (eq? (car prev-f) 'unless))) ; too hard!
			      
			      ;; look for test1 as member of test2 (so we can use test1 as the outer test)
			      ((and (pair? test2)        
				    (eq? (car test2) 'and)
				    (member test1 (cdr test2))
				    (or (eq? f-func 'when)     ; f has to be when or 1-arm if
					(null? (cdddr f)))
				    (or (pair? (cdr fs))        ; if prev-f has false branch, we have to ignore the return value of f
					(eq? (car prev-f) 'when)
					(null? (cdddr prev-f)))
				    (not (side-effect? test2 env))
				    (not (tree-change-member (gather-symbols test1) (cddr prev-f))))
			       (set! test2 (remove test1 test2))
			       (test1-in-test2))
			      
			      ;; look for test2 as member of test1
			      ((and (pair? test1)        
				    (eq? (car test1) 'and)
				    (member test2 (cdr test1))
				    (or (eq? (car prev-f) 'when)     ; prev-f has to be when or 1-arm if
					(null? (cdddr prev-f)))
				    (not (side-effect? test1 env))
				    (not (tree-change-member (gather-symbols test2) (cddr prev-f))))
			       (set! test1 (remove test2 test1))
			       (test2-in-test1))
			      
			      ;; look for some intersection of test1 and test2
			      ((and (pair? test1)
				    (pair? test2)
				    (eq? (car test1) 'and)
				    (eq? (car test2) 'and)
				    (not (side-effect? test1 env))
				    (not (side-effect? test2 env))
				    (not (tree-change-member (gather-symbols test2) (cddr prev-f))))
			       (let ((intersection ())
				     (new-test1 ())
				     (new-test2 ()))
				 (for-each (lambda (tst)
					     (if (member tst test2)
						 (set! intersection (cons tst intersection))
						 (set! new-test1 (cons tst new-test1))))
					   (cdr test1))
				 (for-each (lambda (tst)
					     (if (not (member tst test1))
						 (set! new-test2 (cons tst new-test2))))
					   (cdr test2))
				 (when (pair? intersection)
				   (if (null? new-test1)
				       (if (null? new-test2)
					   (begin
					     (set! test1 `(and ,@(reverse intersection)))
					     (equal-tests))
					   (when (and (or (eq? f-func 'when)
							  (null? (cdddr f)))
						      (or (pair? (cdr fs))
							  (eq? (car prev-f) 'when)
							  (null? (cdddr prev-f))))
					     (set! test1 `(and ,@(reverse intersection)))
					     (set! test2 `(and ,@(reverse new-test2)))
					     (test1-in-test2)))
				       (if (null? new-test2)
					   (when (or (eq? (car prev-f) 'when)
						     (null? (cdddr prev-f)))
					     (set! test2 `(and ,@(reverse intersection)))
					     (set! test1 `(and ,@(reverse new-test1)))
					     (test2-in-test1))
					   
					   (when (and (or (eq? f-func 'when)
							  (null? (cdddr f)))
						      (or (eq? (car prev-f) 'when)
							  (null? (cdddr prev-f))))
					     ;; (... (if (and A B) (f C)) (when (and B C) (g E)) ...) -> (... (when B (when A (f C)) (when C (g E))))
					     (lint-format "perhaps ~A" caller
							  (let ((outer-test (if (null? (cdr intersection))
										(car intersection)
										`(and ,@(reverse intersection)))))
							    (set! new-test1 (if (null? (cdr new-test1)) 
										(car new-test1)
										`(and ,@(reverse new-test1))))
							    (set! new-test2 (if (null? (cdr new-test2)) 
										(car new-test2)
										`(and ,@(reverse new-test2))))
							    (lists->string `(... ,prev-f ,f ...)
									   `(... (when ,outer-test
										   (when ,new-test1
										     ,@(cddr prev-f))
										   (when ,new-test2
										     ,@(cddr f)))
										 ,@(if (null? (cdr fs)) () '(...)))))))))))))))))
		;; --------
		;; check for repeated calls, but only one arg currently can change (more args = confusing separation in code)
		(let ((feq (and (pair? prev-f)
				(pair? f)
				(eq? f-func (car  prev-f))
				(or (equal? (cdr f) (cdr prev-f))
				    (do ((fp (cdr f) (cdr fp))
					 (pp (cdr prev-f) (cdr pp))
					 (i 1 (+ i 1)))
					((or (and (null? pp) 
						  (null? fp))
					     (not (pair? pp))
					     (not (pair? fp))
					     (if (= i repeat-arg)                   ; ignore the arg that's known to be changing
						 (side-effect? (car pp) env)
						 (and (not (equal? (car pp) (car fp)))
						      (or (positive? repeat-arg)
							  (and (set! repeat-arg i)  ; call this one the changer
							       #f)))))
					 (and (null? pp)
					      (null? fp))))))))
		  (if feq
		      (set! repeats (+ repeats 1)))
		  (when (or (not feq)
			    (= ctr (- len 1))) ; this assumes we're not returning the last value?
		    (when (and (> repeats 2)
			       (not (hash-table-ref syntaces (car prev-f)))) ; macros should be ok here if args are constants
		      (let ((fs-end (if (not feq) fs (cdr fs))))

			(if (zero? repeat-arg)		    ; simple case -- all exprs are identical
			    (let ((step 'i))
			      (if (tree-member step prev-f)
				  (set! step (find-unique-name prev-f)))
			      (lint-format "perhaps ~A... ->~%~NC(do ((~A 0 (+ ~A 1))) ((= ~A ~D)) ~A)" caller 
					   (truncated-list->string prev-f)
					   pp-left-margin #\space
					   step step step (+ repeats 1)
					   prev-f))

			    (let ((args ())
				  (constants? #t)
				  (func-name (car prev-f))
				  (new-arg (if (tree-member 'arg prev-f)
					       (find-unique-name prev-f)
					       'arg)))
			      (do ((p start-repeats (cdr p)))
				  ((eq? p fs-end))
				(set! args (cons (list-ref (car p) repeat-arg) args))
				(if constants? (set! constants? (code-constant? (car args)))))
			      
			      (let ((func (if (and (= repeat-arg 1)
						   (null? (cddar start-repeats)))
					      func-name
					      `(lambda (,new-arg)
						 ,(let ((call (copy prev-f)))
						    (list-set! call repeat-arg new-arg)
						    call)))))
				(if constants?
				    (lint-format "perhaps ~A... ->~%~NC(for-each ~S '(~{~S~^ ~}))" caller
						 (truncated-list->string (car start-repeats))
						 pp-left-margin #\space
						 func
						 (map unquoted (reverse args)))
				    (let ((v (var-member func-name env)))
				      (if (or (and (var? v)
						   (memq (var-ftype v) '(define define* lambda lambda*)))
					      (procedure? (symbol->value func-name *e*)))
					  ;; (let () (write-byte 0) (write-byte 1) (write-byte 2) (write-byte 3) (write-byte 4)) ->
					  ;;    (for-each write-byte '(0 1 2 3 4))
					  (lint-format "perhaps ~A... ->~%~NC(for-each ~S (vector ~{~S~^ ~}))" caller 
						       ;; vector rather than list because it is easier on the GC (list copies in s7)
						       (truncated-list->string (car start-repeats))
						       pp-left-margin #\space
						       func
						       (reverse args))
					  (if (not (or (var? v)
						       (macro? (symbol->value func-name *e*))))
					      ;; (let () (writ 0) (writ 1) (writ 2) (writ 3) (writ (* x 2))) -> (for-each writ (vector 0 1 2 3 (* x 2)))
					      (lint-format "assuming ~A is not a macro, perhaps ~A" caller
							   func-name
							   (lists->string (list '... (car start-repeats) '...) 
									  `(for-each ,func (vector ,@(reverse args))))))))))))))
		    (set! repeats 0)
		    (set! repeat-arg 0)
		    (set! start-repeats fs)))
		;; --------

		(if (pair? f)
		    (begin
		      (set! f-len (length f))
		      (if (eq? f-func 'begin)
			  (lint-format "redundant begin: ~A" caller (truncated-list->string f))))
		    (begin
		      (if (symbol? f)
			  (set-ref f caller f env))
		      (set! f-len 0)))

		;; set-car! + set-cdr! here is usually "clever" code assuming eq?ness, so we can't rewrite it using cons
		;;   but copy does not create a new cons... [if at end of body, the return values will differ]
		(when (= f-len prev-len 3)
		  (when (and (memq f-func '(set-car! set-cdr!))  ; ...(set-car! x (car y)) (set-cdr! x (cdr y))... -> (copy y x)
			     (memq (car prev-f) '(set-car! set-cdr!))
			     (not (eq? (car prev-f) f-func))
			     (equal? (cadr f) (cadr prev-f)))
		    (let ((ncar (caddr (if (eq? f-func 'set-car!) f prev-f)))
			  (ncdr (caddr (if (eq? f-func 'set-car!) prev-f f))))
		      (if (and (pair? ncar)
			       (eq? (car ncar) 'car)
			       (pair? ncdr)
			       (eq? (car ncdr) 'cdr)
			       (equal? (cadr ncar) (cadr ncdr)))
			  (lint-format "perhaps ~A~A ~A~A -> ~A" caller 
				       (if (= ctr 0) "" "...") 
				       (truncated-list->string prev-f)
				       (truncated-list->string f)
				       (if (= ctr (- len 1)) "" "...")
				       `(copy ,(cadr ncar) ,(cadr f))))))

		  ;; successive if's that can be combined into case
		  ;;   else in last if could be accomodated as well
		  (when (and (not rewrote-already)
			     (eq? f-func 'if)
			     (eq? (car prev-f) 'if)
			     (pair? (cadr f))
			     (pair? (cadr prev-f))
			     (= (length f) 3)
			     (= (length prev-f) 3)
			     (memq (caadr prev-f) '(eq? eqv? = char=?)) ; not memx
			     (memq (caadr f) '(eq? eqv? = char=?)))
		    (let ((a1 (cadadr prev-f))
			  (a2 (caddr (cadr prev-f)))
			  (b1 (cadadr f))
			  (b2 (caddr (cadr f))))  ; other possibilities are never hit
		      (when (and (equal? a1 b1)
				 (code-constant? a2)
				 (code-constant? b2)
				 (not (tree-change-member (list a1) (cddr prev-f)))) ; or any symbol in a1?
			(set! rewrote-already #t)
			;; (... (if (= x 1) (display y)) (if (= x 2) (f y)) ...) -> (case x ((1) (display y)) ((2) (f y)) ((3) (display z)))
			(lint-format "perhaps ~A" caller
				     (lists->string `(... ,prev-f ,f ...)
						    `(case ,a1
						       ((,(unquoted a2)) ,@(unbegin (caddr prev-f)))
						       ((,(unquoted b2)) ,@(unbegin (caddr f)))
						       ,@(do ((more ())
							      (nfs (cdr fs) (cdr nfs)))
							     ((let ((nf (if (pair? nfs) (car nfs) ())))
								(not (and (pair? nf)
									  (eq? (car nf) 'if)
									  (= (length nf) 3)
									  (pair? (cadr nf))
									  (memq (caadr nf) '(eq? eqv? = char=?))
									  (equal? a1 (cadadr nf))
									  (code-constant? (caddr (cadr nf))))))
							      ;; maybe add (not (tree-change-member (list a1) (cddr last-f)))
							      ;;   but it never is needed
							      (reverse more))
							   (if (pair? nfs)
							       (set! more (cons (cons (list (unquoted (caddr (cadar nfs))))
										      (unbegin (caddar nfs)))
										  more))))))))))
		  (when (and (eq? f-func 'set!)
			     (eq? (car prev-f) 'set!))
		    (let ((arg1 (caddr prev-f))
			  (arg2 (caddr f))
			  (settee (cadr f)))
		      (if (eq? settee (cadr prev-f))
			  (cond ((not (and (pair? arg2)          ; (set! x 0) (set! x 1) -> "this could be omitted: (set! x 0)"
					   (tree-unquoted-member settee arg2)))
				 (if (not (or (side-effect? arg1 env)
					      (side-effect? arg2 env)))
				     (lint-format "this could be omitted: ~A" caller prev-f)))
				
				((and (pair? arg1)               ; (set! x (cons 1 z)) (set! x (cons 2 x)) -> (set! x (cons 2 (cons 1 z)))
				      (pair? arg2)
				      (eq? (car arg1) 'cons)
				      (eq? (car arg2) 'cons)
				      (eq? settee (caddr arg2))
				      (not (eq? settee (cadr arg2))))
				 (lint-format "perhaps ~A ~A -> ~A" caller
					      prev-f f
					      `(set! ,settee (cons ,(cadr arg2) (cons ,@(cdr arg1))))))
				
				((and (pair? arg1)               ; (set! x (append x y)) (set! x (append x z)) -> (set! x (append x y z))
				      (pair? arg2)
				      (eq? (car arg1) 'append)
				      (eq? (car arg2) 'append)
				      (eq? settee (cadr arg1))
				      (eq? settee (cadr arg2))
				      (not (tree-memq settee (cddr arg1)))
				      (not (tree-memq settee (cddr arg2))))
				 (lint-format "perhaps ~A ~A -> ~A" caller
					      prev-f f
					      `(set! ,settee (append ,settee ,@(cddr arg1) ,@(cddr arg2)))))
				
				((and (= (tree-count1 settee arg2 0) 1) ; (set! x y) (set! x (+ x 1)) -> (set! x (+ y 1))
				      (or (not (pair? arg1))
					  (< (tree-leaves arg1) 5)))
				 (lint-format "perhaps ~A ~A ->~%~NC~A" caller 
					      prev-f f pp-left-margin #\space
					      (object->string `(set! ,settee ,(tree-subst arg1 settee arg2))))))
			  
			  (if (and (symbol? (cadr prev-f)) ; (set! x (A)) (set! y (A)) -> (set! x (A)) (set! y x)
				   (pair? arg1)            ;   maybe more trouble than it's worth
				   (equal? arg1 arg2)
				   (not (eq? (car arg1) 'quote))
				   (hash-table-ref no-side-effect-functions (car arg1))
				   (not (tree-unquoted-member (cadr prev-f) arg1))
				   (not (side-effect? arg1 env))
				   (not (maker? arg1)))
			      (lint-format "perhaps ~A" caller (lists->string f `(set! ,settee ,(cadr prev-f)))))))))
		
		(if (< ctr (- len 1)) 
		    (begin		             ; f is not the last form, so its value is ignored
		      (if (and (escape? f env)
			       (pair? (cdr fs)) ; do special case
			       (every? (lambda (arg)
					 (not (and (symbol? arg)
						   (let ((v (var-member arg env)))
						     (and (var? v)
							  (eq? (var-initial-value v) :call/cc))))))
				       (cdr f)))
			  (if (= ctr (- len 2))
			      ;; (let () (error 'oops "an error") #t)
			      (lint-format "~A makes this pointless: ~A" caller
					   (truncated-list->string f)
					   (truncated-list->string (cadr fs)))
			      ;; (begin (stop) (exit 6) (print 4) (stop))
			      (lint-format "~A makes the rest of the body unreachable: ~A" caller
					   (truncated-list->string f)
					   (truncated-list->string (list '... (cadr fs) '...)))))

		      (check-returns caller f env))
		    
		    ;; here f is the last form in the body
		    (when (and (pair? prev-f)
			       (pair? (cdr prev-f)))

		      (case (car prev-f)
			((display write write-char write-byte)
			 (if (and (equal? f (cadr prev-f))
				  (not (side-effect? f env)))
			     ;; (cond ((= x y) y) (else (begin (display x) x)))
			     (lint-format "~A returns its first argument, so this could be omitted: ~A" caller 
					  (car prev-f) (truncated-list->string f))))
		      
			((vector-set! float-vector-set! int-vector-set! byte-vector-set! 
                          string-set! list-set! hash-table-set! let-set!  set-car! set-cdr!)
			 (if (equal? f (list-ref prev-f (- (length prev-f) 1)))
			     ;; (begin (vector-set! x 0 (* y 2)) (* y 2))
			     (lint-format "~A returns the new value, so this could be omitted: ~A" caller 
					  (car prev-f) (truncated-list->string f)))
			 (if (and (pair? f)
				  (pair? (cdr f))
				  (eq? (cadr prev-f) (cadr f))
				  (not (code-constant? (cadr f)))
				  (case (car prev-f)
				    ((vector-set! float-vector-set! int-vector-set!)
				     (memq f-func '(vector-ref float-vector-ref int-vector-ref)))
				    ((list-set!)
				     (eq? f-func 'list-ref))
				    ((string-set!)
				     (eq? f-func 'string-ref))
				    ((set-car!)
				     (eq? f-func 'car))
				    ((set-cdr!)
				     (eq? f-func 'cdr))
				    (else #f))
				  (or (memq f-func '(car cdr)) ; no indices
				      (and (pair? (cddr f))     ; for the others check that indices match
					   (equal? (caddr f) (caddr prev-f))
					   (pair? (cdddr prev-f))
					   (not (pair? (cddddr prev-f)))
					   (not (pair? (cdddr f)))
					   (not (side-effect? (caddr f) env)))))
			     ;; (let ((x (list 1 2))) (set-car! x 3) (car x))
			     (lint-format "~A returns the new value, so this could be omitted: ~A" caller
					  (car prev-f) (truncated-list->string f))))

			((copy)
			 (if (or (and (null? (cddr prev-f))
				      (equal? (cadr prev-f) f))
				 (and (pair? (cddr prev-f))
				      (null? (cdddr prev-f))
				      (equal? (caddr prev-f) f)))
			     (lint-format "~A returns the new value, so ~A could be omitted" caller 
					  (truncated-list->string prev-f)
					  (truncated-list->string f))))
			
			((set! define define* define-macro define-constant define-macro* 
                               defmacro defmacro* define-expansion define-bacro define-bacro*)
			 (cond ((not (and (pair? (cddr prev-f))                 ; (set! ((L 1) 2)) an error, but lint should keep going
					  (or (and (equal? (caddr prev-f) f)    ; (begin ... (set! x (...)) (...))
						   (not (side-effect? f env)))
					      (and (symbol? f)                  ; (begin ... (set! x ...) x)
						   (eq? f (cadr prev-f)))       ; also (begin ... (define x ...) x)
					      (and (not (eq? (car prev-f) 'set!))
						   (pair? (cadr prev-f))        ; (begin ... (define (x...)...) x)
						   (eq? f (caadr prev-f)))))))
			       
			       ((not (memq (car prev-f) '(define define*)))
				(lint-format "~A returns the new value, so this could be omitted: ~A" caller
					     (car prev-f) (truncated-list->string f)))
			       
			       ((symbol? (cadr prev-f))
				(lint-format "perhaps omit ~A and return ~A" caller
					     (cadr prev-f)
					     (caddr prev-f)))
			       
			       ((= (tree-count2 f body 0) 2)
				;; (let () (define (f1 x) (+ x 1)) f1) -> (lambda (x) ...)
				(lint-format "perhaps omit ~A, and change ~A" caller
					     f
					     (lists->string `(,(car prev-f) ,(cadr prev-f) ...)
							    `(,(if (eq? (car prev-f) 'define) 'lambda 'lambda*)
							      ,(cdadr prev-f)
							      ...))))
			       
			       (else (lint-format "~A returns the new value, so this could be omitted: ~A" caller
						  (car prev-f) f)))))))
					; possibly still not right if letrec?
	      
		;; needs f fs prev-f dpy-f dpy-start ctr len
		;;   trap lint-format
		(let ((dpy-case (and (pair? f)
				     (memq f-func '(display write newline write-char write-string))))) ; flush-output-port?
		  (when (and dpy-case
			     (not dpy-start))
		    (set! dpy-f fs)
		    (set! dpy-start ctr))
		  (when (and (integer? dpy-start)
			     (> (- ctr dpy-start) (if dpy-case 1 2))
			     (or (= ctr (- len 1))
				 (not dpy-case)))
		    ;; display sequence starts at dpy-start, goes to ctr (prev-f) unless not dpy-case
		    (let ((ctrl-string "")
			  (args ())
			  (dctr 0)
			  (dpy-last (if (not dpy-case) prev-f f))
			  (op (write-port (car dpy-f)))
			  (exprs (make-list (if dpy-case (- ctr dpy-start -1) (- ctr dpy-start)) ())))
		      
		      (define* (gather-format str (arg :unset))
			(set! ctrl-string (string-append ctrl-string str))
			(unless (eq? arg :unset) (set! args (cons arg args))))
		      
		      (call-with-exit
		       (lambda (done)
			 (for-each
			  (lambda (d)
			    (if (not (equal? (write-port d) op)) 
				(begin 
				  (lint-format "unexpected port change: ~A -> ~A in ~A" caller op (write-port d) d) ; ??
				  (done)))
			    (list-set! exprs dctr d)
			    (set! dctr (+ dctr 1))
			    (gather-format (display->format d))
			    (when (eq? d dpy-last) ; op can be null => send to (current-output-port), return #f or #<unspecified>
			      ;; (begin (display x) (newline) (display y) (newline)) -> (format () "~A~%~A~%" x y)
			      (lint-format "perhaps ~A" caller (lists->string `(... ,@exprs)
									      `(format ,op ,ctrl-string ,@(reverse args))))
			      (done)))
			  dpy-f))))
		    (set! dpy-start #f))
		  (unless dpy-case (set! dpy-start #f)))
		
		(if (and (pair? f)
			 (memq head '(defmacro defmacro* define-macro define-macro* define-bacro define-bacro*))
			 (tree-member 'unquote f))
		    (lint-format "~A probably has too many unquotes: ~A" caller head (truncated-list->string f)))

		(set! prev-f f)
		(set! prev-len f-len)

		(set! lint-current-form f)
		(if (= ctr (- len 1))
		    (set! env (lint-walk caller f env))
		    (begin
		      (set! lint-mid-form f)
		      (let ((e (lint-walk caller f env)))
			(if (and (pair? e)
				 (not (memq (var-name (car e)) '(:lambda :dilambda))))
			    (set! env e)))))
		(set! lint-current-form #f)
		(set! lint-mid-form #f)

		;; need to put off this ref tick until we have a var for it (lint-walk above)
		(when (and (= ctr (- len 1))
			   (pair? f)
			   (pair? (cdr f)))
		  (if (and (pair? (cadr f))
			   (memq f-func '(define define* define-macro define-constant define-macro* define-expansion define-bacro define-bacro*)))
		      (set-ref (caadr f) caller #f env)
		      (if (memq f-func '(defmacro defmacro*))
			  (set-ref (cadr f) caller #f env))))
		))
	    (set! lint-mid-form old-mid-form)
	    (set! lint-current-form old-current-form)))
      env)
    
    
    (define (check-sequence-constant function-name last)
      (let ((seq (if (not (pair? last))
		     last
		     (and (eq? (car last) 'quote)
			  (pair? (cdr last)) ; (quote . 1)
			  (cadr last)))))
	(if (and (sequence? seq)
		 (> (length seq) 0))
	    (begin
	      (lint-format "returns ~A constant: ~A~S" function-name ; (define-macro (m a) `(+ 1 a))
			   (if (pair? seq)
			       (values "a list" "'" seq)
			       (values (prettify-checker-unq (->lint-type last)) "" seq)))
	      (throw 'sequence-constant-done)) ; just report one constant -- the full list is annoying
	    (when (pair? last)
	      (case (car last)

		((begin let let* letrec letrec* when unless with-baffle with-let)
		 (when (pair? (cdr last))
		   (let ((len (length last)))
		     (when (positive? len)
		       (check-sequence-constant function-name (list-ref last (- len 1)))))))
		
		((if)
		 (when (and (pair? (cdr last))
			    (pair? (cddr last)))
		   (check-sequence-constant function-name (caddr last))
		   (if (pair? (cdddr last))
		       (check-sequence-constant function-name (cadddr last)))))
		
		((cond)
		 (for-each (lambda (c)
			     (if (and (pair? c)
				      (pair? (cdr c)))
				 (check-sequence-constant function-name (list-ref c (- (length c) 1)))))
			   (cdr last)))
		
		((case)
		 (when (and (pair? (cdr last))
			    (pair? (cddr last)))
		   (for-each (lambda (c)
			       (if (and (pair? c)
					(pair? (cdr c)))
				   (check-sequence-constant function-name (list-ref c (- (length c) 1)))))
			     (cddr last))))
		
		((do)
		 (if (and (pair? (cdr last))
			  (pair? (cddr last))
			  (pair? (caddr last))
			  (pair? (cdaddr last)))
		     (check-sequence-constant function-name (list-ref (caddr last) (- (length (caddr last)) 1))))))))))
		   
	
    (define (lint-walk-function-body definer function-name args body env)
      ;; walk function body, with possible doc string at the start
      (when (and (pair? body)
		 (pair? (cdr body))
		 (string? (car body)))
	(if *report-doc-strings*
	    (lint-format "old-style doc string: ~S, in s7 use 'documentation:~%~NC~A" function-name
			 (car body) (+ lint-left-margin 4) #\space
			 (lint-pp `(define ,function-name
				     (let ((documentation ,(car body)))
				       (,(if (eq? definer 'define) 'lambda
					     (if (eq? definer 'define*) 'lambda*
						 definer))
					,args
					,@(cdr body)))))))
	(set! body (cdr body))) ; ignore old-style doc-string
      ;; (set! arg ...) never happens as last in body

      ;; but as first in body, it happens ca 100 times
      (if (and (pair? body)
	       (pair? (car body))
	       (eq? (caar body) 'set!)
	       (or (eq? (cadar body) args)
		   (and (pair? args)
			(memq (cadar body) args))))
	  ;; (define (f21 x y) (set! x 3) (+ y 1))
	  (lint-format "perhaps ~A" function-name
		       (lists->string (car body) `(let ((,(cadar body) ,(caddar body))) ...))))
      ;; as first in let of body, maybe a half-dozen
      
      (catch 'sequence-constant-done
	(lambda ()
	  (check-sequence-constant function-name (list-ref body (- (length body) 1))))
	(lambda args #f))

      (lint-walk-body function-name definer body env))

    (define (lint-walk-function definer function-name args body form env)
      ;; check out function arguments (adding them to the current env), then walk its body
      ;; first check for (define (hi...) (ho...)) where ho has no opt args (and try to ignore possible string constant doc string)
      
      (when (eq? definer 'define)
	(let ((bval (if (and (pair? body)
			     (string? (car body)))
			(cdr body)          ; strip away the (old-style) documentation string
			body)))
	  
	  (cond ((not (and (pair? bval)             ; not (define (hi a) . 1)!
			   (pair? (car bval))
			   (null? (cdr bval))
			   (symbol? (caar bval))))) ; not (define (hi) ((if #f + abs) 0))

		((or (equal? args (cdar bval))
		     (and (hash-table-ref reversibles (caar bval))
			  (equal? args (reverse (cdar bval)))))
		 (let* ((cval (caar bval))
			(p (symbol->value cval *e*))
			(ary (arity p)))
		   (if (or (procedure? p)
			   (let ((e (var-member cval env) ))
			     (and e
				  (var? e)
				  (symbol? (var-ftype e))
				  (let ((def (var-initial-value e)) 
					(e-args (var-arglist e)))
				    (and 
				     (pair? def)
				     (memq (var-ftype e) '(define lambda))
				     (or (and (null? args)
					      (null? e-args))
					 (and (symbol? args)
					      (symbol? e-args))
					 (and (pair? args)
					      (pair? e-args)
					      (= (length args) (length e-args)))))))))
		       (lint-format "~A~A could be (define ~A ~A)" function-name 
				    (if (and (procedure? p)
					     (not (= (car ary) (cdr ary)))
					     (not (= (length args) (cdr ary))))
					(format #f "leaving aside ~A's optional arg~P, " cval (- (cdr ary) (length args)))
					"")
				    function-name 
				    function-name 
				    (if (equal? args (cdar bval))
					cval
					(hash-table-ref reversibles (caar bval))))
		       (if (and (null? args)            ; perhaps this can be extended to any equal args
				(null? (cdar bval)))
			   ;; (define (getservent) (getserv)) -> (define getservent getserv)
			   (lint-format "~A could probably be ~A" function-name
					(truncated-list->string form)
					(truncated-list->string `(define ,function-name ,cval)))))))
		
		((and (or (symbol? args)
			  (and (pair? args)
			       (negative? (length args))))
		      (eq? (caar bval) 'apply)
		      (pair? (cdar bval))
		      (symbol? (cadar bval))
		      (not (memq (cadar bval) '(and or)))
		      (pair? (cddar bval))
		      (or (and (eq? args (caddar bval))
			       (null? (cdddar bval)))
			  (and (pair? args)
			       (equal? (cddar bval) (proper-list args)))))
		 ;; (define (f1 . x) (apply + x)) -> (define f1 +)
		 (lint-format "~A could be (define ~A ~A)" function-name function-name function-name (cadar bval)))
		
		((and (hash-table-ref combinable-cxrs (caar bval))
		      (pair? (cadar bval)))
		 ((lambda* (cr arg)
		    (and cr
			 (< (length cr) 5)
			 (pair? args) 
			 (null? (cdr args))
			 (eq? (car args) arg)
			 (let ((f (symbol "c" cr "r")))
			   (if (eq? f function-name)
			       ;; (define (cadddr l) (caddr (cdr l)))
			       (lint-format "this redefinition of ~A is pointless (use (with-let (unlet)...) or #_~A)" definer function-name function-name)
			       ;; (define (f1 x) (cdr (car x))) -> (define f1 cdar)
			       (lint-format "~A could be (define ~A ~A)" function-name function-name function-name f)))))
		  (combine-cxrs (car bval))))
		
		((not (and (memq (caar bval) '(list-ref list-tail))
			   (pair? (cdar bval))
			   (pair? (cddar bval))
			   (pair? args)
			   (eq? (car args) (cadar bval))
			   (null? (cdr args)))))
		
		((eq? (caar bval) 'list-ref)
		 (case (caddar bval)
		   ((0) (lint-format "~A could be (define ~A car)" function-name function-name function-name))
		   ((1) (lint-format "~A could be (define ~A cadr)" function-name function-name function-name))
		   ((2) (lint-format "~A could be (define ~A caddr)" function-name function-name function-name))
		   ((3) (lint-format "~A could be (define ~A cadddr)" function-name function-name function-name))))
		
		(else
		 (case (caddar bval)
		   ((1) (lint-format "~A could be (define ~A cdr)" function-name function-name function-name))
		   ((2) (lint-format "~A could be (define ~A cddr)" function-name function-name function-name))
		   ((3) (lint-format "~A could be (define ~A cdddr)" function-name function-name function-name))
		   ((4) (lint-format "~A could be (define ~A cddddr)" function-name function-name function-name)))))))
      
      (let ((fvar (and (symbol? function-name)
		       (make-fvar :name (if (memq definer '(lambda lambda*)) 
					    :lambda
					    (if (eq? definer 'dilambda) 
						:dilambda 
						function-name))
				  :ftype definer
				  :initial-value form
				  :env env
				  :arglist (if (memq definer '(lambda lambda*))
					       (cadr form)
					       ((if (memq definer '(defmacro defmacro*)) caddr cdadr) form))))))
	(when fvar
	  (let ((fvar-let (cdr fvar)))
	    (set! (fvar-let 'decl)
		  (catch #t
		    (lambda ()
		      (case definer
			((lambda)
			 (set! (fvar-let 'allow-other-keys) #t)
			 (eval (list definer (cadr form) #f)))
			
			((lambda*)
			 (set! (fvar-let 'allow-other-keys) (eq? (last-par (cadr form)) :allow-other-keys))
			 (eval (list definer (copy (cadr form)) #f)))         ; eval can remove :allow-other-keys!
			
			((define*)
			 (set! (fvar-let 'allow-other-keys) (eq? (last-par (cdadr form)) :allow-other-keys))
			 (eval (list definer (cons '_ (copy (cdadr form))) #f)))
			
			((defmacro defmacro*)
			 (set! (fvar-let 'allow-other-keys) (or (not (eq? definer 'defmacro*))
							     (eq? (last-par (caddr form)) :allow-other-keys)))
			 (eval (list definer '_ (caddr form) #f)))
			
			((define-constant)
			 (set! (fvar-let 'allow-other-keys) #t)
			 (eval (list 'define (cons '_ (cdadr form)) #f)))
			
			(else
			 (set! (fvar-let 'allow-other-keys) (or (not (memq definer '(define-macro* define-bacro*)))
							     (eq? (last-par (cdadr form)) :allow-other-keys)))
			 (eval (list definer (cons '_ (cdadr form)) #f)))))
		    (lambda args
		      'error)))))
	
	(if (null? args)
	    (begin
	      (if (memq definer '(define* lambda* defmacro* define-macro* define-bacro*))
		  (lint-format "~A could be ~A"       ; (define* (f1) 32)
			       function-name definer
			       (symbol (substring (symbol->string definer) 0 (- (length (symbol->string definer)) 1)))))
	      (let ((cur-env (if fvar (cons fvar env) env)))
		(let ((nvars (let ((e (lint-walk-function-body definer function-name args body cur-env)))
			       (and (not (eq? e cur-env))
				    (env-difference function-name e cur-env ())))))
		  (if (pair? nvars)
		      (report-usage function-name definer nvars cur-env)))
		cur-env))
	    
	    (if (not (or (symbol? args) 
			 (pair? args)))
		(begin
		  (lint-format "strange ~A parameter list ~A" function-name definer args)
		  env)
		(let ((args-as-vars (if (symbol? args)                            ; this is getting arg names to add to the environment
				    (list (make-var :name args :definer 'parameter))
				    (map
				     (lambda (arg)
				       (if (symbol? arg)
					   (if (memq arg '(:rest :allow-other-keys))
					       (values)                  ; omit :rest and :allow-other-keys
					       (make-var :name arg :definer 'parameter))
					   (if (not (and (pair? arg)
							 (= (length arg) 2)
							 (memq definer '(define* lambda* defmacro* define-macro* define-bacro* definstrument define*-public))))
					       (begin
						 (lint-format "strange parameter for ~A: ~S" function-name definer arg)
						 (values))
					       (begin
						 (if (not (cadr arg)) ; (define* (f4 (a #f)) a)
						     (lint-format "the default argument value is #f in ~A ~A" function-name definer arg))
						 (make-var :name (car arg) :definer 'parameter)))))
				     (proper-list args)))))
		  
		  (let* ((cur-env (cons (make-var :name :let
						  :initial-value form
						  :definer definer)
					(append args-as-vars (if fvar (cons fvar env) env))))
			 (nvars (let ((e (lint-walk-function-body definer function-name args body cur-env)))
				  (and (not (eq? e cur-env))
				       (env-difference function-name e cur-env ())))))
		    (report-usage function-name definer (append (or nvars ()) args-as-vars) cur-env))

		  (when (and (var? fvar)
			     (memq definer '(define lambda define-macro)))
		    ;; look for unused parameters that are passed a value other than #f
		    (let ((set ())
			  (unused ()))
		      (for-each 
		       (lambda (arg-var)
			 (if (zero? (var-ref arg-var))
			     (if (positive? (var-set arg-var))
				 (set! set (cons (var-name arg-var) set))
				 (if (not (memq (var-name arg-var) '(documentation signature iterator?)))
				     (set! unused (cons (var-name arg-var) unused))))))
		       args-as-vars)
		      (when (or (pair? set)
				(pair? unused))
			(let ((proper-args (args->proper-list args)))
			  (let ((sig (var-signature fvar))
				(len (+ (length proper-args) 1)))
			    (if (not sig)
				(set! sig (make-list len #t))
				(if (< (length sig) len)
				    (set! sig (copy sig (make-list len #t)))))
			    (let ((siglist (cdr sig)))
			      (for-each
			       (lambda (arg)
				 (if (memq arg unused)
				     (set-car! siglist 'unused-parameter?)
				     (if (memq arg set)
					 (set-car! siglist 'unused-set-parameter?)))
				 (set! siglist (cdr siglist)))
			       proper-args))
			    (set! (var-signature fvar) sig))))))
		  (if fvar 
		      (cons fvar env)
		      env))))))
    
    
    (define (check-bool-cond caller form c1 c2 env)
      ;; (cond (x #f) (#t #t)) -> (not x)
      ;; c1/c2 = possibly combined, so in (cond (x #t) (y #t) (else #f)), c1: ((or x y) #t), so -> (or x y)
      (and (pair? c1) 
	   (= (length c1) 2) 
	   (pair? c2) 
	   (pair? (cdr c2))
	   (memq (car c2) '(#t else))
	   (or (and (boolean? (cadr c1))
		    (or (and (null? (cddr c2))
			     (boolean? (cadr c2))
			     (not (equal? (cadr c1) (cadr c2))) ; handled elsewhere
			     (lint-format "perhaps ~A" caller 
					  (lists->string form (if (eq? (cadr c1) #t) 
								  (car c1)
								  (simplify-boolean `(not ,(car c1)) () () env)))))
			(and (not (cadr c1))   ; (cond (x #f) (else y)) -> (and (not x) y)
			     (let ((cc1 (simplify-boolean `(not ,(car c1)) () () env)))
			       (lint-format "perhaps ~A" caller 
					    (lists->string form 
							   (if (null? (cddr c2)) 
							       `(and ,cc1 ,(cadr c2))
							       `(and ,cc1 (begin ,@(cdr c2))))))))
			(and (pair? (car c1))  ; (cond ((null? x) #t) (else y)) -> (or (null? x) y)
			     (eq? (return-type (caar c1) env) 'boolean?)
			     (lint-format "perhaps ~A" caller
					  (lists->string form 
							 (if (null? (cddr c2)) 
							     `(or ,(car c1) ,(cadr c2))
							     `(or ,(car c1) (begin ,@(cdr c2)))))))))
	       (and (boolean? (cadr c2))
		    (null? (cddr c2))
		    (not (equal? (cadr c1) (cadr c2)))
		    ;; (cond ((= 3 (length eq)) (caddr eq)) (else #f)) -> (and (= 3 (length eq)) (caddr eq))
		    (lint-format "perhaps ~A" caller
				 (lists->string form
						(if (cadr c2)
						    `(or (not ,(car c1)) ,(cadr c1))
						    (if (and (pair? (car c1))
							     (eq? (caar c1) 'and))
							(append (car c1) (cdr c1))
							`(and ,@c1)))))))))
    
    (define (case-branch test eqv-select exprs)
      (case (car test)
	((eq? eqv? = equal? char=?)
	 (if (equal? eqv-select (cadr test))
	     `((,(unquoted (caddr test))) ,@exprs)
	     `((,(unquoted (cadr test))) ,@exprs)))
	
	((memq memv member)
	 `(,(unquoted (caddr test)) ,@exprs))
	
	((not)
	 `((#f) ,@exprs))
	
	((null?)
	 `((()) ,@exprs))
	
	((eof-object?)
	 `((#<eof>) ,@exprs))
	
	((zero?)
	 `((0 0.0) ,@exprs))
	
	((boolean?)
	 `((#t #f) ,@exprs))
	
	((char-ci=?)
	 (if (equal? eqv-select (cadr test))
	     `(,(list (caddr test) (other-case (caddr test))) ,@exprs)
	     `(,(list (cadr test) (other-case (cadr test))) ,@exprs)))
	
	(else 
	 `(,(map (lambda (p)
		   (case (car p)
		     ((eq? eqv? = equal? char=?)  
		      (unquoted ((if (equal? eqv-select (cadr p)) caddr cadr) p)))
		     ((memq memv member) (apply values (caddr p)))
		     ((not)              #f)
		     ((null?)            ())
		     ((eof-object?)      #<eof>)
		     ((zero?)            (values 0 0.0))
		     ((boolean?)         (values #t #f))
		     ((char-ci=?)   
		      (if (equal? eqv-select (cadr p)) 
			  (values (caddr p) (other-case (caddr p)))
			  (values (cadr p) (other-case (cadr p)))))
		     (else               (error "oops"))))
		 (cdr test))
	   ,@exprs))))

    (define (cond->case eqv-select new-clauses)
      `(case ,eqv-select 
	 ,@(map (lambda (clause)
		  (let ((test (car clause))
			(exprs (cdr clause)))
		    (if (null? exprs)                   ; cond returns the test result if no explicit results
			(set! exprs (list #t)))         ;   but all tests here return a boolean, and we win only if #t?? (memx is an exception)
		    (if (memq test '(else #t))
			`(else ,@exprs)
			(case-branch test eqv-select exprs))))
		new-clauses)))
    
    (define (eqv-code-constant? x)
      (or (number? x)
	  (char? x)
	  (and (pair? x)
	       (eq? (car x) 'quote)
	       (or (symbol? (cadr x))
		   (and (not (pair? (cadr x)))
			(eqv-code-constant? (cadr x)))))
	  (memq x '(#t #f () #<unspecified> #<undefined> #<eof>))))

    (define (cond-eqv? clause eqv-select or-ok)
      (if (not (pair? clause))
	  (memq clause '(else #t))
	  ;; it's eqv-able either directly or via memq/memv, or via (or ... eqv-able clauses)
	  ;;   all clauses involve the same (eventual case) selector
	  (case (car clause)
	    ((eq? eqv? = equal? char=? char-ci=?)
	     (if (eqv-code-constant? (cadr clause))
		 (equal? eqv-select (caddr clause))
		 (and (eqv-code-constant? (caddr clause))
		      (equal? eqv-select (cadr clause)))))
	    
	    ((memq memv member)
	     (and (equal? eqv-select (cadr clause))
		  (pair? (caddr clause))
		  (eq? (caaddr clause) 'quote)
		  (or (not (eq? (car clause) 'member))
		      (every? (lambda (x)
				(or (number? x)
				    (char? x)
				    (symbol? x)
				    (memq x '(#t #f () #<unspecified> #<undefined> #<eof>))))
			      (cdr (caddr clause))))))
	    ((or)
	     (and or-ok
		  (every? (lambda (p)
			    (cond-eqv? p eqv-select #f))
			  (cdr clause))))

	    ((not null? eof-object? zero? boolean?)
	     (equal? eqv-select (cadr clause)))
	    
	    (else #f))))
    
    (define (find-constant-exprs caller vars body)
      (if (or (tree-set-member '(call/cc call-with-current-continuation lambda lambda* define define* 
				 define-macro define-macro* define-bacro define-bacro* define-constant define-expansion)
			       body)
	      (let set-walk ((tree body)) ; generalized set! causes confusion
		(and (pair? tree)
		     (or (and (eq? (car tree) 'set!)
			      (pair? (cdr tree))
			      (pair? (cadr tree)))
			 (set-walk (car tree))
			 (set-walk (cdr tree))))))
	  ()
	  (let ((refs (let ((vs (out-vars caller vars body)))
			(remove-if (lambda (v)
				     (or (assq v vars)        ; vars = do-loop steppers
					 (memq v (cadr vs)))) ; (cadr vs) = sets
				   (car vs))))
		;; refs are the external variables accessed in the do-loop body
		;;   that are not set or shadowed or changed (vector-set! etc)
		(constant-exprs ()))

	    (let expr-walk ((tree body))
	      (when (pair? tree)
		(if (let all-ok? ((tree tree))
		      (if (symbol? tree)
			  (memq tree refs)
			  (or (not (pair? tree))
			      (eq? (car tree) 'quote)
			      (and (hash-table-ref no-side-effect-functions (car tree))
				   (or (not (hash-table-ref syntaces (car tree)))
				       (memq (car tree) '(if begin cond or and unless when)))
				   (not (hash-table-ref makers (car tree)))
				   (list? (cdr tree))
				   (every? all-ok? (cdr tree))))))
		    (if (not (or (eq? (car tree) 'quote) (member tree constant-exprs)))
			(set! constant-exprs (cons tree constant-exprs)))
		    (begin
		      (if (pair? (car tree))
			  (expr-walk (car tree)))
		      (when (pair? (cdr tree))
			(let ((f (cdr tree)))
			  (case (car f)
			    ((case)
			     (when (and (pair? (cdr f)) 
					(pair? (cddr f)))
			       (expr-walk (cadr f))
			       (for-each (lambda (c) 
					   (expr-walk (cdr c)))
					 (cddr f))))
			    ((letrec letrec*)
			     (when (pair? (cddr f))
			       (for-each (lambda (c)
					   (if (and (pair? c) 
						    (pair? (cdr c)))
					       (expr-walk (cadr c))))
					 (cadr f))
			       (expr-walk (cddr f))))
			    ((let let*)
			     (when (pair? (cddr f))
			       (if (symbol? (cadr f))
				   (set! f (cdr f)))
			       (for-each (lambda (c)
					   (if (and (pair? c) 
						    (pair? (cdr c)))
					       (expr-walk (cadr c))))
					 (cadr f))
			       (expr-walk (cddr f))))
			    ((do)
			     (when (and (list? (cadr f))
					(list? (cddr f)) 
					(pair? (cdddr f)))
			       (for-each (lambda (c) 
					   (if (pair? (cddr c)) 
					       (expr-walk (caddr c))))
					 (cadr f))
			       (expr-walk (cdddr f))))
			    (else (for-each expr-walk f)))))))))
	    (when (pair? constant-exprs)
	      (set! constant-exprs (remove-if (lambda (p)
						(or (null? (cdr p))
						    (and (null? (cddr p))
							 (memq (car p) '(not -))
							 (symbol? (cadr p)))
						    (tree-unquoted-member 'port-line-number p)))
					      constant-exprs)))
	    constant-exprs)))
    
    (define (partition-form start len)
      (let ((ps (make-vector len))
	    (qs (make-vector len)))
	(do ((i 0 (+ i 1))
	     (p start (cdr p)))
	    ((= i len))
	  (set! (ps i) (cadar p))
	  (set! (qs i) (reverse (cadar p))))

	(let ((header-len (length (ps 0))))
	  (let ((trailer-len header-len)
		(result-min-len header-len))
	    (do ((i 1 (+ i 1)))
		((= i len))
	      (set! result-min-len (min result-min-len (length (ps i))))
	      (do ((k 1 (+ k 1))
		   (p (cdr (ps i)) (cdr p))
		   (f (cdr (ps 0)) (cdr f)))
		  ((or (= k header-len)
		       (not (pair? p))
		       (not (equal? (car p) (car f))))
		   (set! header-len k)))
	      (do ((k 0 (+ k 1))
		   (q (qs i) (cdr q))
		   (f (qs 0) (cdr f)))
		  ((or (= k trailer-len)
		       (not (pair? q))
		       (not (equal? (car q) (car f))))
		   (set! trailer-len k))))
	    
	    (if (= result-min-len header-len)
		(begin
		  (set! header-len (- header-len 1))
		  (set! trailer-len 0)))
	    (if (<= result-min-len (+ header-len trailer-len))
		(set! trailer-len (- result-min-len header-len 1)))

	    (values header-len trailer-len result-min-len)))))

    (define (one-call-and-dots body) ; body is unchanged here, so it's not interesting
      (if (< (tree-leaves body) 30)
	  (if (null? (cdr body))
	      body
	      (list (car body) '...))
	  (if (pair? (car body))
	      (list (list (caar body) '...))
	      (list (car body) '...))))

    (define (replace-redundant-named-let caller form outer-name outer-args inner)
      (when (proper-list? outer-args)  ; can be null
	(let ((inner-name (cadr inner))
	      (inner-args (caddr inner))
	      (inner-body (cdddr inner)))
	  (do ((p outer-args (cdr p))
	       (a inner-args (cdr a)))
	      ((or (null? p)
		   (not (pair? a))
		   (not (pair? (car a)))
		   (and (not (eq? (car p) (caar a)))
			(tree-memq (car p) inner-body)))
	       ;; args can be reversed, but rarely match as symbols
	       (when (and (null? p)
			  (or (null? a)
			      (and (null? (cdr a))
				   (code-constant? (cadar a)))))
		 (let* ((args-match (do ((p outer-args (cdr p))
					 (a inner-args (cdr a)))
					((or (null? p)
					     (not (eq? (car p) (caar a)))
					     (not (eq? (caar a) (cadar a))))
					 (null? p))))
			(args-aligned (and (not args-match)
					   (do ((p outer-args (cdr p))
						(a inner-args (cdr a)))
					       ((or (null? p)
						    (not (eq? (car p) (cadar a))))
						(null? p))))))
		   (when (or args-match args-aligned)
		     (let ((definer (if (null? a) 'define 'define*))
			   (extras (if (and (pair? a)
					    (quoted-null? (cadar a)))
				       (list (list (caar a) ()))
				       a)))
		       ;;     (define (f61 x) (let loop ((y x)) (if (positive? y) (loop (- y 1)) 0))) -> (define (f61 y) (if (positive? y) (f61 (- y 1)) 0))
		       (lint-format "~A ~A" caller
				    (if (null? a) "perhaps" "a toss-up -- perhaps")
				    (lists->string form
						   `(,definer (,outer-name 
							       ,@(if args-match 
								     outer-args
								     (do ((result ())
									  (p outer-args (cdr p))
									  (a inner-args (cdr a)))
									 ((null? p)
									  (reverse result))
								       (set! result (cons (caar a) result))))
							       ,@extras)
						      ,@(tree-subst outer-name inner-name inner-body)))))))))))))
    
    (define (set!? form env)
      (and *report-any-!-as-setter* ; (inc! x) when inc! is unknown, assume it sets x
	   (symbol? (car form))
	   (pair? (cdr form))
	   (or (symbol? (cadr form))
	       (and (pair? (cddr form))
		    (symbol? (caddr form))))
	   (not (var-member (car form) env))
	   (not (hash-table-ref built-in-functions (car form)))
	   (let ((str (symbol->string (car form))))
	     (char=? (string-ref str (- (length str) 1)) #\!))))

    (define (set-target name form env)
      (and (pair? form)
	   (or (and (pair? (cdr form))
		    (or (eq? (cadr form) name)    ; (pop! x)
			(and (pair? (cddr form))  ; (push! y x)
			     (eq? (caddr form) name)))
		    (or (eq? (car form) 'set!)    ; (set! x y)
			(set!? form env)))
	       (set-target name (car form) env)
	       (set-target name (cdr form) env))))


    (define (check-definee caller sym form env)
      (cond ((keyword? sym)               ; (define :x 1)
	     (lint-format "keywords are constants ~A" caller sym))
	    
	    ((and (eq? sym 'pi)           ; (define pi (atan 0 -1))
		  (member (caddr form) '((atan 0 -1)
					 (acos -1)
					 (* 2 (acos 0))
					 (* 4 (atan 1))
					 (* 4 (atan 1 1)))))
	     (lint-format "~A is one of its many names, but pi is a predefined constant in s7" caller (caddr form)))
	    
	    ((constant? sym)              ; (define most-positive-fixnum 432)
	     (lint-format "~A is a constant in s7: ~A" caller sym form))
	    
	    ((eq? sym 'quote)
	     (lint-format "either a stray quote, or a real bad idea: ~A" caller (truncated-list->string form)))
	    
	    ((pair? sym)
	     (check-definee caller (car sym) form env))
	    
	    ((let ((v (var-member sym env)))
	       (and (var? v)
		    (eq? (var-definer v) 'define-constant)
		    (not (equal? (caddr form) (var-initial-value v)))))
	     => (lambda (v)
		  (let ((line (if (and (pair? (var-initial-value v))
				       (positive? (pair-line-number (var-initial-value v))))
				  (format #f "(line ~D): " (pair-line-number (var-initial-value v)))
				  "")))
		    (lint-format "~A in ~A is already a constant, defined ~A~A" caller sym
				 (truncated-list->string form)
				 line
				 (truncated-list->string (var-initial-value v))))))))
				   
    (define binders (let ((h (make-hash-table)))
		      (for-each
		       (lambda (op)
			 (set! (h op) #t))
		       '(let let* letrec letrec* do
			     lambda lambda* define define* 
			     call/cc call-with-current-continuation 
			     define-macro define-macro* define-bacro define-bacro* define-constant define-expansion
			     load eval eval-string require))
		      h))
    
    (define lint-let-reduction-factor 3) ; maybe make this a global switch -- the higher this number, the fewer let-reduction suggestions
    
    (define walker-functions
      (let ((h (make-hash-table)))

	;; ---------------- define ----------------		  
	(let ()
	  (define (define-walker caller form env)
	    (if (< (length form) 2)
		(begin
		  (lint-format "~S makes no sense" caller form)
		  env)
		(let ((sym (cadr form))
		      (val (cddr form))
		      (head (car form)))

		  (if (symbol? sym)
		      (begin
			(check-definee caller sym form env)
			
			(if (memq head '(define define-constant define-envelope 
					  define-public define*-public defmacro-public define-inlinable 
					  define-integrable define^))
			    (let ((len (length form)))
			      (if (not (= len 3))  ;  (define a b c)
				  (lint-format "~A has ~A value~A?"
					       caller (truncated-list->string form)
					       (if (< len 3)
						   (values "no" "")
						   (values "too many" "s")))))
			    (lint-format "~A is messed up" caller (truncated-list->string form)))
			
			(if (not (pair? val))
			    env
			    (begin
			      (if (and (null? (cdr val))
				       (equal? sym (car val))) ; (define a a)
				  (lint-format "this ~A is either not needed, or is an error: ~A" caller head (truncated-list->string form)))
			      
			      (if (not (pair? (car val)))
				  (begin
				    (if (not (memq caller '(module cond-expand)))
					(cond ((hash-table-ref other-identifiers sym)
					       => (lambda (p)
						    (lint-format "~A is used before it is defined: ~A" caller sym form)))))
				    (cons (make-var :name sym :initial-value (car val) :definer head) env))
				  
				  (let ((e (lint-walk (if (and (pair? (car val))
							       (eq? (caar val) 'letrec))
							  'define sym)
						      (car val) env)))
				    (if (or (not (pair? e))
					    (eq? e env)
					    (not (memq (var-name (car e)) '(:lambda :dilambda)))) ; (define x (lambda ...))
					(cons (make-var :name sym :initial-value (car val) :definer head) env)
					(begin
					  (set! (var-name (car e)) sym)
					  
					  (let ((val (caddr form)))
					    (when (and (eq? (car val) 'lambda) ; (define sym (lambda args (let name...))), let here happens rarely
						       (proper-list? (cadr val))
						       (pair? (caddr val))
						       (null? (cdddr val))
						       (eq? (caaddr val) 'let)
						       (symbol? (cadr (caddr val))))
					      (replace-redundant-named-let caller form sym (cadr val) (caddr val))))
					  
					  ;; (define x (letrec ((y (lambda...))) (lambda (...) (y...)))) -> (define (x...)...)
					  (let* ((let-form (cdaddr form))
						 (var (and (pair? (car let-form))
							   (null? (cdar let-form)) ; just one var in let/rec
							   (caar let-form))))
					    ;; let-form here can be cdr of (lambda...) or (let|letrec ... lambda)
					    (when (and (pair? var)
						       (symbol? (car var))
						       (pair? (cdr let-form))
						       (pair? (cadr let-form))
						       (null? (cddr let-form))     ; just one form in the let/rec
						       (pair? (cdr var))
						       (pair? (cadr var))
						       (pair? (cdadr var))
						       (eq? (caadr var) 'lambda)    ; var is lambda
						       (proper-list? (cadadr var))) ; it has no rest arg
					      (let ((body (cadr let-form)))
						(when (and (eq? (car body) 'lambda)     ; let/rec body is lambda calling var
							   (proper-list? (cadr body))   ; rest args are a headache
							   (pair? (caddr body)))    ; (lambda (...) (...) where car is letrec func name
						  (if (eq? (caaddr body) (car var))
						      (lint-format "perhaps ~A" caller
								   (lists->string form
										  `(define (,sym ,@(cadr body))
										     (let ,(car var)
										       ,(map list (cadadr var) (cdaddr body))
										       ,@(cddadr var)))))
						      (let ((call (find-call (car var) (caddr body))))
							(when (and (pair? call)       ; inner lambda body is (...some-expr...(sym...) ...)
								   (= (tree-count1 (car var) (caddr body) 0) 1))
							  (let ((new-call `(let ,(car var)
									     ,(map list (cadadr var) (cdr call))
									     ,@(cddadr var))))
							    (lint-format "perhaps ~A" caller
									 (lists->string form
											`(define (,sym ,@(cadr body))
											   ,(tree-subst new-call call
													(caddr body)))))))))))))
					  (when (and *report-function-stuff*
						     (pair? (caddr (var-initial-value (car e)))))
					    (hash-table-set! equable-closures (caaddr (var-initial-value (car e)))
							     (cons (car e) (or (hash-table-ref equable-closures (caaddr (var-initial-value (car e)))) ()))))
					  e))))))) ; symbol? sym
		      
		      ;; not (symbol? sym)
		      (if (and (pair? sym) ; cadr form
			       (pair? val) ; cddr form
			       (not (pair? (car sym)))) ; pair would indicate a curried func or something equally stupid
			  (let ((outer-args (cdr sym))
				(outer-name (car sym)))
			    
			    (cond ((not *report-forward-functions*))
				  ;; need to ignore macro usages here -- this happens ca 20000 times!
				  ((hash-table-ref other-identifiers (car sym))
				   => (lambda (p)
					(lint-format "~A is used before it is defined" caller (car sym)))))
			    
			    (check-definee caller (car sym) form env)
			    
			    (when (pair? (car val))
			      (when (eq? (caar val) 'let)
				(when (pair? (cadar val))
				  (do ((inner-vars (cadar val))
				       (p outer-args (cdr p)))
				      ((not (pair? p)))
				    (cond ((assq (car p) inner-vars) =>
					   (lambda (v)
					     (if (eq? (cadr v) (car p))
						 ;; (define (f70 a b) (let ((a a) (b b)) (+ a b)))
						 (lint-format "in ~A this let binding is pointless: ~A" caller
							      (truncated-list->string form)
							      v)))))))
				
				;; define + redundant named-let -- sometimes rewrites to define*
				(when (and (symbol? (cadar val))
					   (null? (cdr val)))
				  (replace-redundant-named-let caller form outer-name outer-args (car val))))
			      
			      ;; perhaps this block should be on a *report-* switch --
			      ;;   it translates some internal defines into named lets
			      ;;   (or just normal lets, etc)
			      ;; this is not redundant given the walk-body translations because here
			      ;;   we have the outer parameters and can check those against the inner ones
			      ;;   leading (sometimes) to much nicer rewrites.
			      (when (and (eq? (caar val) 'define) ; define* does not happen here
					 (pair? (cdr val))
					 (pair? (cadar val)))     ; inner define (name ...)
				(let ((inner-name (caadar val))
				      (inner-args (cdadar val))
				      (inner-body (cddar val))
				      (outer-body (cdddr form)))
				  (when (and (symbol? inner-name)
					     (proper-list? inner-args)
					     (pair? (car outer-body))
					     (= (tree-count1 inner-name outer-body 0) 1))
				    (let ((call (find-call inner-name outer-body)))
				      (when (pair? call)
					(set! last-rewritten-internal-define (car val))
					(let ((new-call (if (tree-memq inner-name inner-body)
							    (if (and (null? inner-args)
								     (null? outer-args))
								(if (null? (cdr inner-body))
								    (car (tree-subst outer-name inner-name inner-body))
								    `(begin ,@(tree-subst outer-name inner-name inner-body)))
								`(let ,inner-name
								   ,(if (null? inner-args) () (map list inner-args (cdr call)))
								   ,@inner-body))
							    (if (or (null? inner-args)
								    (and (equal? inner-args outer-args)
									 (equal? inner-args (cdr call))))
								(if (null? (cdr inner-body))
								    (car (tree-subst outer-name inner-name inner-body))
								    `(begin ,@(tree-subst outer-name inner-name inner-body)))
								`(let ,(map list inner-args (cdr call))
								   ,@inner-body)))))
					  ;; (define (f11 a b) (define (f12 a b) (if (positive? a) (+ a b) b)) (f12 a b)) ->
					  ;;    (define (f11 a b) (if (positive? a) (+ a b) b))
					  (lint-format "perhaps ~A" caller
						       (lists->string form 
								      `(,head ,sym 
									      ,@(let ((p (tree-subst new-call call outer-body)))
										  (if (and (pair? p)
											   (pair? (car p))
											   (eq? (caar p) 'begin))
										      (cdar p)
										      p))))))))))))
			    (when (pair? outer-args)
			      (if (repeated-member? (proper-list outer-args) env)
				  (lint-format "~A parameter is repeated: ~A" caller head (truncated-list->string sym)))
			      
			      (cond ((memq head '(define* define-macro* define-bacro* define*-public))
				     (check-star-parameters outer-name outer-args env))
				    ((list-any? keyword? outer-args)
				     (lint-format "~A parameter can't be a keyword: ~A" caller outer-name sym))
				    ((memq 'pi outer-args)
				     (lint-format "~A parameter can't be a constant: ~A" caller outer-name sym)))
			      
			      ;; look for built-in names used as parameter names and used as functions internally(!)
			      ;;   this requires a tree walker to ignore (for example) (let loop ((string string))...)
			      (for-each (lambda (p)
					  (let ((par (if (pair? p) (car p) p)))
					    (when (or (hash-table-ref built-in-functions par)
						      (hash-table-ref syntaces par))
					      (let ((call (call-with-exit 
							   (lambda (return)
							     (let loop ((tree (cddr form)))
							       (if (pair? tree)
								   (if (eq? (car tree) par)
								       (return tree)
								       (case (car tree)
									 ((quote) #f)
									 ((let let*)
									  (if (pair? (cdr tree))
									      (if (symbol? (cadr tree))
										  (if (not (tree-memq par (caddr tree)))
										      (loop (cdddr tree)))
										  (if (not (tree-memq par (cadr tree)))
										      (loop (cddr tree))))))
									 ((letrec letrec*)
									  (if (and (pair? (cdr tree))
										   (not (tree-memq par (cadr tree))))
									      (loop (cddr tree))))
									 ((do)
									  (if (and (pair? (cdr tree))
										   (pair? (cddr tree))
										   (not (tree-memq par (cadr tree))))
									      (loop (cdddr tree))))
									 (else 
									  (if (pair? (cdr tree))
									      (for-each loop (cdr tree)))
									  (if (pair? (car tree))
									      (loop (car tree))))))))))))
						(if (and (pair? call)
							 (pair? (cdr call))
							 (not (eq? par (cadr call))))
						    (lint-format* caller ; (define (f50 abs) (abs -1))
								  (string-append (object->string outer-name) "'s parameter " (symbol->string par))
								  (string-append " is called " (truncated-list->string call))
								  ": find a less confusing parameter name!"))))))
					outer-args))
			    
			    (when (and (eq? head 'define-macro)
				       (pair? val)
				       (null? (cdr val)))
			      (let ((body (car val)))
				(if (and (null? outer-args)  ; look for C macros translated as define-macro! -- this happens a lot sad to say
					 (or (not (symbol? body))
					     (keyword? body))
					 (or (not (pair? body))
					     (and (eq? (car body) 'quote)
						  (not (symbol? (cadr body)))
						  (or (not (pair? (cadr body)))
						      (eq? (caadr body) 'quote)))
					     (not (or (memq (car body) '(quote quasiquote list cons append))
						      (tree-set-member '(#_{list} #_{apply_values} #_{append}) body)))))
				    (lint-format "perhaps ~A or ~A" caller 
						 (lists->string form `(define ,outer-name ,(unquoted (car val))))
						 (truncated-list->string `(define (,outer-name) ,(unquoted (car val))))))

				(when (pair? body)
				  (case (car body)
				    ((#_{list})
				     (when (and (quoted-symbol? (cadr body))
						(proper-list? outer-args))
				       (if (and (equal? (cddr body) outer-args)
						(or (not (hash-table-ref syntaces (cadadr body))) ; (define-macro (x y) `(lambda () ,y))
						    (memq (cadadr body) '(set! define))))
					   (lint-format "perhaps ~A" caller
							(lists->string form `(define ,outer-name ,(cadadr body))))
					   (if (and (not (hash-table-ref syntaces (cadadr body)))
						    (not (any-macro? (cadadr body) env))
						    (every? (lambda (a)
							      (or (code-constant? a)
								  (and (memq a outer-args)
								       (= (tree-count1 a (cddr body) 0) 1))))
							    (cddr body)))  
					       ;; marginal -- there are many debatable cases here
					       (lint-format "perhaps ~A" caller
							    (lists->string form `(define (,outer-name ,@outer-args)
										   (,(cadadr body) ,@(map unquoted (cddr body)))))))))
				     (let ((pargs (args->proper-list outer-args)))
				       (for-each (lambda (p)
						   (if (and (pair? p)
							    (eq? (car p) 'quote)
							    (pair? (cdr p))
							    (pair? (cadr p))
							    (tree-set-member pargs (cadr p)))
						       (lint-format "missing comma? ~A" caller form)))
						 (cdr body))))
				    
				    ((quote)
				     ;; extra comma (unquote) is already caught elsewhere
				     (if (and (pair? (cdr body))
					      (pair? (cadr body))
					      (tree-set-member (args->proper-list outer-args) (cadr body)))
					 (lint-format "missing comma? ~A" caller form)))))))
			    
			    (if (and (eq? head 'definstrument)
				     (string? (car val)))
				(set! val (cdr val)))
			    
			    (if (keyword? outer-name)
				env
				(lint-walk-function head outer-name outer-args val form env)))
			  
			  (begin ; not (and (pair? sym)...)
			    (lint-format "strange form: ~A" head (truncated-list->string form))
			    (when (and (pair? sym)
				       (pair? (car sym)))
			      (let ((outer-args (cdr sym))
				    (outer-name (if (eq? head 'define*) (remove :optional (car sym)) (car sym))))
				(if (symbol? (car outer-name))
				    ;; perhaps a curried definition -- as a public service, we'll rewrite the dumb thing
				    (begin
				      (lint-format "perhaps ~A" caller
						   (lists->string form `(,head ,outer-name 
									       (lambda ,outer-args 
										 ,@(cddr form)))))
				      (lint-walk-function head (car outer-name) (cdr outer-name) val form env)) ;val=(cddr form) I think
				    (when (pair? (car outer-name))
				      (if (symbol? (caar outer-name))
					  (begin
					    (lint-format "perhaps ~A" caller
							 (lists->string form `(,head ,(car outer-name) 
										     (lambda ,(cdr outer-name)
										       (lambda ,outer-args
											 ,@(cddr form))))))
					    (lint-walk-function head (caar outer-name) (cdar outer-name) val form env))
					  (when (and (pair? (caar outer-name))
						     (symbol? (caaar outer-name)))
					    (lint-format "perhaps ~A" caller
							 (lists->string form `(,head ,(caar outer-name) 
										     (lambda ,(cdar outer-name)
										       (lambda ,(cdr outer-name)
											 (lambda ,outer-args
											   ,@(cddr form)))))))
					    (lint-walk-function head (caaar outer-name) (cdaar outer-name) val form env)))))))
			    env))))))

	  (for-each (lambda (op)
		      (hash-table-set! h op define-walker))
		    '(define define* define-constant 
		      define-macro define-macro* define-bacro define-bacro* define-expansion 
		      definstrument defanimal define-envelope        ; for clm
		      define-public define*-public defmacro-public define-inlinable 
		      define-integrable define^)))                   ; these give more informative names in Guile and scmutils (MIT-scheme))


	;; ---------------- dilambda ----------------
	(let ()
	  (define (dilambda-walker caller form env)
	    ;(format *stderr* "~A~%" form)
	    (let ((len (length form)))
	      (if (not (= len 3))
		  (begin
		    (lint-format "dilambda takes two arguments: ~A" caller (truncated-list->string form))
		    env)
		  (let ((getter (cadr form))
			(setter (caddr form)))
		    (lint-walk caller setter env)
		    (let ((e (lint-walk caller getter env))) ; goes to lint-walk-function -> :lambda as first in e
		      (if (and (pair? e)
			       (eq? (var-name (car e)) :lambda))
			  (set! (var-name (car e)) :dilambda))
		      e)))))

	  (hash-table-set! h 'dilambda dilambda-walker))


	;; ---------------- lambda ----------------		  
	(let ()
	  (define (lambda-walker caller form env)
	    (let ((len (length form))
		  (head (car form)))
	      (if (< len 3)
		  (begin
		    (lint-format "~A is messed up in ~A" caller head (truncated-list->string form))
		    env)
		  (let ((args (cadr form)))
		    (when (list? args)
		      (let ((arglen (length args)))
			(if (null? args)
			    (if (eq? head 'lambda*)             ; (lambda* ()...) -> (lambda () ...)
				(lint-format "lambda* could be lambda ~A" caller form))
			    (begin ; args is a pair             ; (lambda (a a) ...)
			      (let ((val (caddr form)))
				(if (and (pair? val)
					 (eq? (car val) 'let)
					 (pair? (cadr val)))
				    (do ((inner-vars (cadr val))
					 (p (cadr form) (cdr p)))
					((not (pair? p)))
				      (cond ((assq (car p) inner-vars) =>
					     (lambda (v)
					       (if (eq? (cadr v) (car p))
						   (lint-format "in ~A this let binding is pointless: ~A" caller
								(truncated-list->string form)
								v))))))))
			    
			      (if (repeated-member? (proper-list args) env)
				  (lint-format "~A parameter is repeated: ~A" caller head (truncated-list->string args)))
			      (if (eq? head 'lambda*)           ; (lambda* (a :b) ...)
				  (check-star-parameters head args env)
				  (if (list-any? keyword? args) ; (lambda (:key) ...)
				      (lint-format "lambda arglist can't handle keywords (use lambda*)" caller)))))
			
			(when (and (eq? head 'lambda)           ; (lambda () (f)) -> f, (lambda (a b) (f a b)) -> f
				   (not (eq? caller 'case-lambda))    
				   (= len 3)
				   (>= arglen 0)) ; not a dotted list
			  (let ((body (caddr form)))
			    (cond ((not (and (pair? body)
					     (symbol? (car body))
					     (not (memq (car body) '(and or))))))

				  ((equal? args (cdr body))
				   ;; (lambda (a b) (> a b)) -> >
				   (lint-format "perhaps ~A" caller (lists->string form (car body))))
				  
				  ((equal? (reverse args) (cdr body))
				   (let ((rf (hash-table-ref reversibles (car body))))
				     ;; (lambda (a b) (> b a)) -> <
				     (if rf (lint-format "perhaps ~A" caller (lists->string form rf)))))
				  
				  ((and (= arglen 1)
					(hash-table-ref combinable-cxrs (car body)))
				   ((lambda* (cr arg) ; lambda* not lambda because combine-cxrs might return just #f
				      (and cr
					   (< (length cr) 5) 
					   (eq? (car args) arg)
					   ;; (lambda (x) (cdr (cdr (car x)))) -> cddar
					   (lint-format "perhaps ~A" caller 
							(lists->string form (symbol "c" cr "r")))))
				    (combine-cxrs body))))))))
		    
		    (if (and (or (symbol? args)                 ; (lambda args (apply f args)) -> f
				 (and (pair? args)              ; (lambda #\a ...) !
				      (negative? (length args))))
			     (eq? head 'lambda)
			     (not (eq? caller 'case-lambda))    
			     (= len 3))
			(let ((body (caddr form)))
			  (if (and (pair? body)
				   (eq? (car body) 'apply)
				   (pair? (cdr body))
				   (symbol? (cadr body))
				   (not (memq (cadr body) '(and or)))
				   (pair? (cddr body))
				   (or (eq? args (caddr body))
				       (and (pair? args)
					    (equal? (cddr body) (proper-list args)))))
			      ;; (lambda args (apply + args)) -> +
			      (lint-format "perhaps ~A" caller (lists->string form (cadr body))))))
		    
		    (lint-walk-function head caller args (cddr form) form env)
		    ;; not env as return value here -- return the lambda+old env via lint-walk-function
		    ))))
	  
	  (hash-table-set! h 'lambda lambda-walker)
	  (hash-table-set! h 'lambda* lambda-walker))
	
	
	;; ---------------- set! ----------------	
	(let ()
	 (define (set-walker caller form env)
	   (if (not (= (length form) 3))
	       (begin
		 (lint-format "set! has too ~A arguments: ~S" caller (if (> (length form) 3) "many" "few") form)
		 env)
	       (let ((settee (cadr form))
		     (setval (caddr form)))
		 (if (symbol? setval)
		     (set-ref setval caller form env))
		 (let ((result (lint-walk caller setval env)))
		   (if (symbol? settee)
		       (if (constant? settee)  ; (set! pi 3)
			   (lint-format "can't set! ~A (it is a constant)" caller (truncated-list->string form))
			   (let ((v (var-member settee env)))
			     (if (and (var? v)
				      (eq? (var-definer v) 'define-constant))
				 (let ((line (if (and (pair? (var-initial-value v))
						      (positive? (pair-line-number (var-initial-value v))))
						 (format #f "(line ~D): " (pair-line-number (var-initial-value v)))
						 "")))
				   (lint-format "can't set! ~A in ~A (it is a constant: ~A~A)" caller settee
						(truncated-list->string form)
						line
						(truncated-list->string (var-initial-value v)))))))
		       (if (not (pair? settee))  ; (set! 3 1)
			   (lint-format "can't set! ~A" caller (truncated-list->string form))
			   (begin
			     (if (memq (car settee) '(vector-ref list-ref string-ref hash-table-ref))
				 ;; (set! (vector-ref v 0) 3)
				 (lint-format "~A as target of set!~A" caller (car settee) (truncated-list->string form)))
			     (lint-walk caller settee env) ; this counts as a reference since it's by reference so to speak
			     
			     ;; try type check (dilambda signatures)
			     (when (symbol? (car settee))
			       (let ((f (symbol->value (car settee) *e*)))
				 (when (dilambda? f)
				   (let ((sig (procedure-signature (procedure-setter f)))
					 (settee-len (length settee)))
				     (when (and (pair? sig)
						(positive? settee-len)
						(pair? (list-tail sig settee-len)))
				       (let ((checker (list-ref sig settee-len))
					     (arg-type (->lint-type setval)))
					 (when (and (symbol? checker)
						    (not (compatible? checker arg-type)))
					   ;; (set! (print-length) "asd")
					   (lint-format "~A: new value should be a~A ~A: ~S: ~A" 
							caller (car settee)
							(if (char=? (string-ref (format #f "~A" checker) 0) #\i) "n" "")
							checker arg-type
							(truncated-list->string form)))))))))
			     (set! settee (do ((sym (car settee) (car sym)))
					      ((not (pair? sym)) sym))))))
		   
		   (if (symbol? (cadr form)) ; see do directly above -- sets settee so we have to go back to (cadr form)
		       (set-set (cadr form) caller form env)
		       (if (and (pair? (cadr form))
				(symbol? settee))
			   (set-ref settee caller `(implicit-set ,@(cdr form)) env)))
		   
		   (if (equal? (cadr form) setval) ; not settee here!   ;  (set! a a)
		       (lint-format "pointless set! ~A" caller (truncated-list->string form)))

		   (when (and (pair? setval)           
			      (symbol? settee))
		     (case (car setval)
		       ((if)                        ; (set! x (if y x 1)) -> (if (not y) (set! x 1))
			(if (= (length setval) 4)
			    (if (eq? settee (caddr setval))
				(lint-format "perhaps ~A" caller 
					     (lists->string form `(if (not ,(cadr setval)) (set! ,settee ,(cadddr setval)))))
				(if (eq? settee (cadddr setval))
				    (lint-format "perhaps ~A" caller 
						 (lists->string form `(if ,(cadr setval) (set! ,settee ,(caddr setval)))))))))
		       
		       ((or)                        ; (set! x (or x y)) -> (if (not x) (set! x y))
			(if (and (= (length setval) 3)   ;    the other case here is not improved by using 'if
				 (eq? settee (cadr setval)))
			    (lint-format "perhaps ~A" caller 
					 (lists->string form `(if (not ,settee) (set! ,settee ,(caddr setval)))))))
		       
		       ((and)
			(if (= (length setval) 3)   ; (set! x (and x y)) -> (if x (set! x y))
			    (if (eq? settee (cadr setval))
				(lint-format "perhaps ~A" caller 
					     (lists->string form `(if ,settee (set! ,settee ,(caddr setval)))))
				(if (eq? settee (caddr setval))
				    (lint-format "perhaps ~A" caller 
						 (lists->string form `(if (not ,(cadr setval)) (set! ,settee #f))))))))))
		   result))))
	 (hash-table-set! h 'set! set-walker))
	
	
	;; ---------------- quote ----------------		  
	(let ()
	 (define (quote-walker caller form env)
	   (let ((len (length form)))
	     (if (negative? len)
		 (lint-format "stray dot in quote's arguments? ~S" caller form)
		 (if (not (= len 2))
		     (lint-format "quote has too ~A arguments: ~S" caller (if (> len 2) "many" "few") form)
		     (let ((arg (cadr form)))
		       (if (pair? arg)
			   (if (> (length arg) 8)
			       (hash-table-set! big-constants arg (+ 1 (or (hash-table-ref big-constants arg) 0))))
			   (unless (or (>= quote-warnings 20)
				       (and (symbol? arg) 
					    (not (keyword? arg))))
			     (set! quote-warnings (+ quote-warnings 1))           ; (char? '#\a)
			     (lint-format "quote is not needed here: ~A~A" caller ; this is by far the most common message from lint
					  (truncated-list->string form)
					  (if (= quote-warnings 20) "; will ignore this error henceforth." ""))))))))
	   env)
	 (hash-table-set! h 'quote quote-walker))
	
	;; ---------------- if ----------------
	(let ()
	 (define (if-walker caller form env)
	   (let ((len (length form)))
	     (if (> len 4)
		 (lint-format "if has too many clauses: ~A" caller (truncated-list->string form))
		 (if (< len 3)
		     (lint-format "if has too few clauses: ~A" caller (truncated-list->string form))
		     (let ((test (cadr form))
			   (true (caddr form))
			   (false (if (= len 4) (cadddr form) 'no-false))
			   (expr (simplify-boolean (cadr form) () () env))
			   (suggestion made-suggestion)
			   (true-op (and (pair? (caddr form)) (caaddr form)))
			   (false-op (and (= len 4) (pair? (cadddr form)) (car (cadddr form)))))

		       (if (eq? false #<unspecified>)
			   (lint-format "this #<unspecified> is redundant: ~A" caller form))
		       
		       (if (and (symbol? test)
				(pair? true)
				(memq test true))
			   (and-incomplete form 'if test true env)
			   (when (pair? test)
			     (if (and (eq? (car test) 'not)
				      (symbol? (cadr test))
				      (pair? false)
				      (memq (cadr test) false))
				 (and-incomplete form 'if2 (cadr test) false env))
			     (if (and (hash-table-ref bools (car test))
				      (pair? true))
				 (if (member (cadr test) true)
				     (and-forgetful form 'if test true env)
				     (do ((p true (cdr p)))
					 ((or (not (pair? p))
					      (and (pair? (car p))
						   (member (cadr test) (car p))))
					  (if (pair? p)
					      (and-forgetful form 'if test (car p) env)))))
				 (if (and (eq? (car test) 'not)
					  (pair? (cadr test))
					  (pair? false)
					  (hash-table-ref bools (caadr test)))
				     (if (member (cadadr test) false)
					 (and-forgetful form 'if2 (cadr test) false env)
					 (do ((p false (cdr p)))
					     ((or (not (pair? p))
						  (and (pair? (car p))
						       (member (cadadr test) (car p))))
					      (if (pair? p)
						  (and-forgetful form 'if2 (cadr test) (car p) env)))))))))

		       (when (and (pair? true)
				  (pair? false)
				  (not (memq true-op (list 'quote {list})))
				  (not (any-macro? true-op env))
				  (or (not (hash-table-ref syntaces true-op))
				      (memq true-op '(let let* set! and or begin)))
				  (pair? (cdr true)))

			 (define (tree-subst-eq new old tree) 
			   ;; tree-subst above substitutes every occurence of 'old with 'new, so we check
			   ;;   in advance that 'old only occurs once in the tree (via tree-count1).  Here
			   ;;   'old may occur any number of times, but we want to change it only once,
			   ;;   so we keep the actual pointer to it and use eq?.  (This assumes no shared code?)
			   (cond ((eq? old tree)
				  (cons new (cdr tree)))
				 ((not (pair? tree)) 
				  tree)
				 ((eq? (car tree) 'quote)
				  (copy-tree tree))
				 (else (cons (tree-subst-eq new old (car tree))
					     (tree-subst-eq new old (cdr tree))))))
			 
			 ;; maybe move the unless before this 
			 (let ((diff (let differ-in-one ((p true)
							 (q false))
				       (and (pair? p)
					    (pair? q)
					    (if (equal? (car p) (car q))
						(differ-in-one (cdr p) (cdr q))
						(and (equal? (cdr p) (cdr q))
						     (or (and (pair? (car p))
							      (not (eq? (caar p) 'quote))
							      (pair? (car q))
							      (not (eq? (caar q) 'quote))
							      (differ-in-one (car p) (car q)))
							 (list p (list (car p) (car q))))))))))
			   (if (pair? diff)
			       (unless (or (and (equal? true-op (caadr diff)) ; (if x (+ y 1) (- y 1)) -- are we trying to keep really simple stuff out?
						(or (hash-table-ref syntaces true-op)
						    (hash-table-ref syntaces false-op))
						(any? pair? (cdr true)))      ; (if x (set! y (+ x 1)) (set! y 1))
					   (and (eq? true-op 'set!)           ; (if x (set! y w) (set! z w))
						(equal? (caar diff) (cadr true))))
				 (let ((subst-loc (car diff)))
				   ;; for let/let* if tree-subst position can't affect the test, just subst, else save test first
				   ;;   named let diff in args gets no hits
				   (if (memq true-op '(let let*))
				       (if (not (or (symbol? (cadr true))         ; assume named let is moving an if outside the loop
						    (eq? subst-loc (cdr true)))) ;   avoid confusion about the vars list
					   (let ((vars (cadr true)))
					     ;; (if x (let ((y (abs x))) (display z) y) (let ((y (log x))) (display z) y)) -> (let ((y ((if x abs log) x))) (display z) y)
					     (lint-format "perhaps ~A" caller 
							  (lists->string form
									 (if (and (pair? vars)
										  (case true-op
										    ((let)  (tree-memq subst-loc vars))
										    ((let*) (tree-memq subst-loc (car vars)))
										    (else #f)))
									     (tree-subst-eq `(if ,expr ,@(cadr diff)) subst-loc true)
									     `(let ((_1_ ,expr))
										,(tree-subst-eq `(if _1_ ,@(cadr diff)) subst-loc true)))))))
				       
				       ;; also not any-macro? (car true|false) probably
				       ;; (if x (set! y #t) (set! y #f)) -> (set! y x)
				       (lint-format "perhaps ~A" caller 
						    (lists->string form
								   (cond ((eq? true-op (caadr diff))  ; very common!
									  ;; (if x (f y) (g y)) -> ((if x f g) y)
									  ;; but f and g can't be or/and unless there are no expressions
									  ;;   I now like all of these -- originally found them odd: CL influence!
									  (if (equal? true-op test)
									      `((or ,test ,false-op) ,@(cdr true))
									      `((if ,test ,true-op ,false-op) ,@(cdr true))))
									 
									 ((and (eq? (caadr diff) #t)
									       (not (cadadr diff)))
									  ;; (if x (set! y #t) (set! y #f)) -> (set! y x)
									  (tree-subst-eq test subst-loc true))
									 
									 ((and (not (caadr diff))
									       (eq? (cadadr diff) #t))
									  ;; (if x (set! y #f) (set! y #t)) -> (set! y (not x))
									  (tree-subst-eq (simplify-boolean `(not ,expr) () () env)
											 subst-loc true))
									 
									 ((equal? (caadr diff) test)
									  ;; (if x (set! y x) (set! y 21)) -> (set! y (or x 21))
									  (tree-subst-eq (simplify-boolean `(or ,@(cadr diff)) () () env)
											 subst-loc true))
									 
									 ((or (memq true-op '(set! begin and or))
									      (let list-memq ((a subst-loc) (lst true))
										(and (pair? lst)
										     (or (eq? a lst)
											 (list-memq a (cdr lst))))))
									  ;; (if x (set! y z) (set! y w)) -> (set! y (if x z w))
									  ;; true op moved out, if expr moved in
									  ;;  (if A (and B C) (and B D)) -> (and B (if A C D))
									  ;;  here differ-in-one means that preceding/trailing stuff must subst-loc exactly
									  (tree-subst-eq `(if ,expr ,@(cadr diff)) subst-loc true))
									 
									 ;; paranoia... normally the extra let is actually not needed,
									 ;;   but it's very hard to distinguish the bad cases
									 (else 
									  `(let ((_1_ ,expr))
									     ,(tree-subst-eq `(if _1_ ,@(cadr diff)) subst-loc true)))))))))
			       ;; else not pair? diff
			       (unless (memq true-op '(let let*))
				 ;; differ-in-trailers can (sometimes) take advantage of values
				 (let ((enddiff (let differ-in-trailers ((p true)
									 (q false)
									 (c 0))
						  (and (pair? p)
						       (pair? q)
						       (if (equal? (car p) (car q))
							   (differ-in-trailers (cdr p) (cdr q) (+ c 1))
							   (and (> c 1)
								(let ((op (if (memq true-op '(and or + * begin max min)) true-op 'values)))
								  (list p 
									(if (null? (cdr p)) (car p) `(,op ,@p))
									(if (null? (cdr q)) (car q) `(,op ,@q))))))))))
				   
				   ;; (if A (+ B C E) (+ B D)) -> (+ B (if A (+ C E) D))
				   ;; if p/q null, don't change because for example
				   ;;   (if A (or B C) (or B C D F)) can't be (or B C (if A ...))
				   ;;   but if this were not and/or, it could be (+ B (if A C (values C D F)))
				   (if (pair? enddiff)
				       (lint-format "perhaps ~A" caller
						    (lists->string form (tree-subst `((if ,expr ,@(cdr enddiff))) (car enddiff) true)))
				       
				       ;; differ-in-headers looks for equal trailers
				       ;;   (if A (+ B B E C) (+ D D E C)) -> (+ (if A (+ B B) (+ D D)) E C)
				       ;;   these are not always (read: almost never) an improvement
				       (when (and (eq? true-op false-op)
						  (not (eq? true-op 'values))
						  (or (not (eq? true-op 'set!))
						      (equal? (cadr true) (cadr false))))
					 (let ((headdiff (let differ-in-headers ((p (cdr true))
										 (q (cdr false))
										 (c 0)
										 (rp ())
										 (rq ()))
							   (and (pair? p)
								(pair? q)
								(if (equal? p q)
								    (and (> c 0) ; once case is handled elsewhere?
									 (list p (reverse rp) (reverse rq)))
								    (differ-in-headers (cdr p) (cdr q)
										       (+ c 1)
										       (cons (car p) rp) (cons (car q) rq)))))))
					   (when (pair? headdiff)
					     (let ((op (if (memq true-op '(and or + * begin max min)) true-op 'values)))
					       (let ((tp (if (null? (cdadr headdiff)) 
							     (caadr headdiff)
							     `(,op ,@(cadr headdiff))))
						     (tq (if (null? (cdaddr headdiff)) 
							     (caaddr headdiff) 
							     `(,op ,@(caddr headdiff)))))
						 ;; (if A (+ B B E C) (+ D D E C)) -> (+ (if A (+ B B) (+ D D)) E C)
						 (lint-format "perhaps ~A" caller
							      (lists->string form 
									     `(,true-op 
									       (if ,expr ,tp ,tq)
									       ,@(car headdiff)))))))))))))))
		       ;;    (when (and (pair? true)...)
		       ;; end tree-subst section
		       
		       (unless (= last-if-line-number line-number)
			 (do ((iff form (cadddr iff))
			      (iffs 0 (+ iffs 1)))
			     ((not (and (<= iffs 2)
					(pair? iff)
					(= (length iff) 4)
					(eq? (car iff) 'if)))
			      (when (or (> iffs 2)
					(and (= iffs 2)
					     (pair? iff)
					     (= (length iff) 3)
					     (eq? (car iff) 'if)))
				(set! last-if-line-number line-number)
				;; (if a b (if c d (if e f g))) -> (cond (a b) (c d) (e f) (else g))
				(lint-format "perhaps use cond: ~A" caller
					     (lists->string form 
							    `(cond ,@(do ((iff form (cadddr iff))
									  (clauses ()))
									 ((not (and (pair? iff)
										    (= (length iff) 4)
										    (eq? (car iff) 'if)))
									  (append (reverse clauses)
										  (if (and (pair? iff)
											   (= (length iff) 3)
											   (eq? (car iff) 'if))
										      `((,(cadr iff) ,@(unbegin (caddr iff))))
										      `((else ,@(unbegin iff))))))
								       (set! clauses (cons (cons (cadr iff) (unbegin (caddr iff))) clauses))))))))))
		       
		       (if (never-false test)
			   (lint-format "if test is never false: ~A" caller (truncated-list->string form))
			   (if (and (never-true test) true) ; complain about (if #f #f) later
			       ;; (if #f x y)
			       (lint-format "if test is never true: ~A" caller (truncated-list->string form))))
		       
		       (cond ((side-effect? test env))

			     ((or (equal? test true)               ; (if x x y) -> (or x y)
				  (equal? expr true))
			      (lint-format "perhaps ~A" caller 
					   (lists->string form 
							  (simplify-boolean (if (eq? false 'no-false)
										`(or ,expr #<unspecified>)
										`(or ,expr ,false))
									    () () env))))
			     ((or (equal? test `(not ,true))       ; (if x (not x) y) -> (and (not x) y)
				  (equal? `(not ,test) true))      ; (if (not x) x y) -> (and x y)
			      (lint-format "perhaps ~A" caller 
					   (lists->string form 
							  (simplify-boolean (if (eq? false 'no-false)
										`(and ,true #<unspecified>)
										`(and ,true ,false))
									    () () env))))
			     ((or (equal? test false)              ; (if x y x) -> (and x y)
				  (equal? expr false))
			      (lint-format "perhaps ~A" caller 
					   (lists->string form (simplify-boolean `(and ,expr ,true) () () env))))
			     
			     ((or (equal? `(not ,test) false)      ; (if x y (not x)) -> (or (not x) y)
				  (equal? test `(not ,false)))     ; (if (not x) y x) -> (or x y)
			      (lint-format "perhaps ~A" caller 
					   (lists->string form (simplify-boolean `(or ,false ,true) () () env)))))
		       
		       (when (= len 4)
			 (when (and (pair? true)
				    (eq? true-op 'if))
			   (let ((true-test (cadr true))
				 (true-true (caddr true)))
			     (if (= (length true) 4)
				 (let ((true-false (cadddr true)))
				   (if (equal? expr (simplify-boolean `(not ,true-test) () () env))
				       ;; (if a (if (not a) B C) A) -> (if a C A)
				       (lint-format "perhaps ~A" caller
						    (lists->string form `(if ,expr ,true-false ,false))))
				   (if (equal? expr true-test)
				       ;; (if x (if x z w) y) -> (if x z y)
				       (lint-format "perhaps ~A" caller
						    (lists->string form `(if ,expr ,true-true ,false))))
				   (if (equal? false true-false)
				       ;; (if a (if b B A) A) -> (if (and a b) B A)
				       (lint-format "perhaps ~A" caller 
						    (lists->string form 
								   (simplify-boolean
								    (if (not false)
									`(and ,expr ,true-test ,true-true)
									`(if (and ,expr ,true-test) ,true-true ,false))
								    () () env)))
				       (if (equal? false true-true)
					   ;; (if a (if b A B) A) -> (if (and a (not b)) B A)
					   (lint-format "perhaps ~A" caller 
							(lists->string form 
								       (simplify-boolean
									(if (not false)
									    `(and ,expr (not ,true-test) ,true-false)
									    `(if (and ,expr (not ,true-test)) ,true-false ,false))
									() () env)))))
				 
				   ;; (if a (if b d e) (if c d e)) -> (if (if a b c) d e)? reversed does not happen.
				   ;; (if a (if b d) (if c d)) -> (if (if a b c) d)
				   ;; (if a (if b d e) (if (not b) d e)) -> (if (eq? (not a) (not b)) d e)
				   (when (and (pair? false)
					      (eq? false-op 'if)
					      (= (length false) 4)
					      (not (equal? true-test (cadr false)))
					      (equal? (cddr true) (cddr false)))
				     (let ((false-test (cadr false)))
				       (lint-format "perhaps ~A" caller
						    (lists->string form
								   (cond ((and (pair? true-test)
									       (eq? (car true-test) 'not)
									       (equal? (cadr true-test) false-test))
									  `(if (not (eq? (not ,expr) ,true-test))
									       ,@(cddr true)))

									 ((and (pair? false-test)
									       (eq? (car false-test) 'not)
									       (equal? true-test (cadr false-test)))
									  `(if (eq? (not ,expr) ,false-test)
									       ,@(cddr true)))

									 ((> (+ (tree-leaves expr)
										(tree-leaves true-test)
										(tree-leaves false-test))
									     12)
									  `(let ((_1_ (if ,expr ,true-test ,false-test)))
									     (if _1_ ,@(cddr true))))

									 (else
									  `(if (if ,expr ,true-test ,false-test) ,@(cddr true)))))))))
				 (begin ; (length true) != 4
				   (if (equal? expr (simplify-boolean `(not ,true-test) () () env))
				       (lint-format "perhaps ~A" caller  ; (if a (if (not a) B) A) -> (if (not a) A)
						    (lists->string form `(if (not ,expr) ,false))))
				   (if (equal? expr true-test)           ; (if x (if x z) w) -> (if x z w)
				       (lint-format "perhaps ~A" caller 
						    (lists->string form `(if ,expr ,true-true ,false))))
				   (if (equal? false true-true)          ; (if a (if b A) A)
				       (lint-format "perhaps ~A" caller
						    (let ((nexpr (simplify-boolean `(or (not ,expr) ,true-test) () () env)))
						      (lists->string form `(if ,nexpr ,false)))))))))
			 
			 (when (pair? false)
			   (case false-op
			     ((cond)                 ; (if a A (cond...)) -> (cond (a  A) ...)
			      (lint-format "perhaps ~A" caller (lists->string form `(cond (,expr ,true) ,@(cdr false)))))
			     
			     ((if)
			      (when (= (length false) 4)
				(let ((false-test (cadr false))
				      (false-true (caddr false))
				      (false-false (cadddr false)))
				  (if (equal? true false-true)
				      ;; (if a A (if b A B)) -> (if (or a b) A B)
				      (lint-format "perhaps ~A" caller 
						   (if (and (pair? false-false)
							    (eq? (car false-false) 'if)
							    (equal? true (caddr false-false)))
						       (lists->string form 
								      (let ((nexpr (simplify-boolean
										    `(or ,expr ,false-test ,(cadr false-false))
										    () () env)))
									`(if ,nexpr ,true ,@(cdddr false-false))))
						       (if true
							   (let ((nexpr (simplify-boolean `(or ,expr ,false-test) () () env)))
							     (lists->string form `(if ,nexpr ,true ,false-false)))
							   (lists->string form 
									  (simplify-boolean 
									   `(and (not (or ,expr ,false-test)) ,false-false)
									   () () env)))))
				      (if (equal? true false-false)
					  ;; (if a A (if b B A)) -> (if (or a (not b)) A B)
					  (lint-format "perhaps ~A" caller 
						       (if true
							   (let ((nexpr (simplify-boolean `(or ,expr (not ,false-test)) () () env)))
							     (lists->string form `(if ,nexpr ,true ,false-true)))
							   (lists->string form 
									  (simplify-boolean
									   `(and (not (or ,expr (not ,false-test))) ,false-true)
									   () () env))))))))
			      (if (and (pair? true)
				       (eq? true-op 'if)
				       (= (length true) 3)
				       (= (length false) 3)
				       (equal? (cddr true) (cddr false)))
				  ;; (if a (if b d) (if c d)) -> (if (if a b c) d)
				  (lint-format "perhaps ~A" caller
					       (lists->string form
							      (if (> (+ (tree-leaves expr)
									(tree-leaves (cadr true))
									(tree-leaves (cadr false)))
								     12)
								  `(let ((_1_ (if ,expr ,(cadr true) ,(cadr false))))
								     (if _1_ ,@(cddr true)))
								  `(if (if ,expr ,(cadr true) ,(cadr false)) ,@(cddr true)))))))

			     ((map)  ; (if (null? x) () (map abs x)) -> (map abs x)
			      (if (and (pair? test)
				       (eq? (car test) 'null?)
				       (or (null? true)
					   (equal? true (cadr test)))
				       (equal? (cadr test) (caddr false))
				       (or (null? (cdddr false))
					   (not (side-effect? (cdddr false) env))))
				  (lint-format "perhaps ~A" caller (lists->string form false))))
			     
			     ((case)
			      (if (and (pair? expr)
				       (cond-eqv? expr (cadr false) #t))
				  ;; (if (eof-object? x) 32 (case x ((#\\a) 3) (else 4))) -> (case x ((#<eof>) 32) ((#\\a) 3) (else 4))
				  (lint-format "perhaps ~A" caller
					       (lists->string form `(case ,(cadr false)
								      ,(case-branch expr (cadr false) (list true))
								      ,@(cddr false))))))))
			 ) ; (= len 4)
		       
		       (if (pair? false)
			   (let ((false-test (and (pair? (cdr false)) (cadr false))))
			     (if (and (eq? false-op 'if)   ; (if x 3 (if (not x) 4)) -> (if x 3 4)
				      (pair? (cdr false))
				      (not (side-effect? test env)))
				 (if (or (equal? test false-test) 
					 (equal? expr false-test))
				     (lint-format "perhaps ~A" caller (lists->string form `(if ,expr ,true ,@(cdddr false))))
				     (if (and (pair? false-test)
					      (eq? (car false-test) 'not)
					      (or (equal? test (cadr false-test))
						  (equal? expr (cadr false-test))))
					 (lint-format "perhaps ~A" caller (lists->string form `(if ,expr ,true ,(caddr false)))))))
			     
			     (if (and (eq? false-op 'if)   ; (if test0 expr (if test1 expr)) -> if (or test0 test1) expr) 
				      (null? (cdddr false))   ; other case is dealt with above
				      (equal? true (caddr false)))
				 (let ((test1 (simplify-boolean `(or ,expr ,false-test) () () env)))
				   (lint-format "perhaps ~A" caller (lists->string form `(if ,test1 ,true ,@(cdddr false)))))))
			   
			   (when (and (eq? false 'no-false)                         ; no false branch
				      (pair? true))
			     (when (pair? test)
			       (let ((test-op (car test)))
				 ;; the min+max case is seldom hit, and takes about 50 lines
				 (when (and (memq test-op '(< > <= >=))
					    (null? (cdddr test)))
				   (let ((rel-arg1 (cadr test))
					 (rel-arg2 (caddr test)))

				     ;; (if (< x y) (set! x y) -> (set! x (max x y))
				     (if (eq? true-op 'set!)    
					 (let ((settee (cadr true))
					       (setval (caddr true)))
					   (if (and (member settee test)
						    (member setval test)) ; that's all there's room for
					       (let ((f (if (equal? settee (if (memq test-op '(< <=)) rel-arg1 rel-arg2)) 'max 'min)))
						 (lint-format "perhaps ~A" caller
							      (lists->string form `(set! ,settee (,f ,@(cdr true))))))))
					 
					 ;; (if (<= (list-ref ind i) 32) (list-set! ind i 32)) -> (list-set! ind i (max (list-ref ind i) 32))
					 (if (memq true-op '(list-set! vector-set!))
					     (let ((settee (cadr true))   
						   (index (caddr true))
						   (setval (cadddr true)))
					       (let ((mx-op (if (and (equal? setval rel-arg1)
								     (eqv? (length rel-arg2) 3)
								     (equal? settee (cadr rel-arg2))
								     (equal? index (caddr rel-arg2)))
								(if (memq test-op '(< <=)) 'min 'max)
								(and (equal? setval rel-arg2)
								     (eqv? (length rel-arg1) 3)
								     (equal? settee (cadr rel-arg1))
								     (equal? index (caddr rel-arg1))
								     (if (memq test-op '(< <=)) 'max 'min)))))
						 (if mx-op
						     (lint-format "perhaps ~A" caller
								  (lists->string form `(,true-op ,settee ,index (,mx-op ,@(cdr test))))))))))))))
			     
			     (if (eq? (car true) 'if) ; (if test0 (if test1 expr)) -> (if (and test0 test1) expr)
				 (cond ((null? (cdddr true))
					(let ((test1 (simplify-boolean `(and ,expr ,(cadr true)) () () env)))
					  (lint-format "perhaps ~A" caller (lists->string form `(if ,test1 ,(caddr true))))))
				       
				       ((equal? expr (cadr true))
					(lint-format "perhaps ~A" caller (lists->string form true)))
				       
				       ((equal? (cadr true) `(not ,expr))
					(lint-format "perhaps ~A" caller 
						     (lists->string form (cadddr true)))))
				 
				 (if (memq true-op '(when unless))    ; (if test0 (when test1 expr...)) -> (when (and test0 test1) expr...)
				     (let ((test1 (simplify-boolean (if (eq? true-op 'when)
									`(and ,expr ,(cadr true))
									`(and ,expr (not ,(cadr true))))
								    () () env)))
				       ;; (if (and (< x 1) y) (when z (display z) x)) -> (when (and (< x 1) y z) (display z) x)
				       (lint-format "perhaps ~A" caller 
						    (lists->string form 
								   (if (and (pair? test1)
									    (eq? (car test1) 'not))
								       `(unless ,(cadr test1) ,@(cddr true))
								       `(when ,test1 ,@(cddr true))))))))))
		       (if (and (pair? test)
				(memq (car test) '(< <= > >= =))       ; (if (< x y) x y) -> (min x y)
				(null? (cdddr test))
				(member false test)
				(member true test))
			   (if (eq? (car test) '=)                     ; (if (= x y) y x) -> y [this never happens]
			       (lint-format "perhaps ~A" caller (lists->string form false))
			       (let ((f (if (equal? (cadr test) (if (memq (car test) '(< <=)) true false))
					    'min 'max)))
				 (lint-format "perhaps ~A" caller (lists->string form `(,f ,true ,false))))))
		       
		       (cond ((eq? expr #t) ; (if #t #f) -> #f
			      (lint-format "perhaps ~A" caller (lists->string form true)))
			     
			     ((not expr)
			      (if (eq? false 'no-false)
				  (if true                             ; (if #f x) as a kludgey #<unspecified>
				      (lint-format "perhaps ~A" caller (lists->string form #<unspecified>)))
				  ;; (if (negative? (gcd x y)) a b) -> b
				  (lint-format "perhaps ~A" caller (lists->string form false))))
			     
			     ((not (equal? true false))
			      (if (boolean? true)
				  (if (boolean? false) ; !  (if expr #t #f) turned into something less verbose
				      ;; (if x #f #t) -> (not x)
				      (lint-format "perhaps ~A" caller 
						   (lists->string form (if true 
									   expr 
									   (simplify-boolean `(not ,expr) () () env))))
				      (when (= suggestion made-suggestion)
					;; (if x #f y) -> (and (not x) y)
					(lint-format "perhaps ~A" caller 
						     (lists->string form (if true
									     (if (eq? false 'no-false)
										 expr
										 (simplify-boolean `(or ,expr ,false) () () env))
									     (simplify-boolean 
									      (if (eq? false 'no-false)
										  `(not ,expr)
										  `(and (not ,expr) ,false))
									      () () env))))))
				  (if (and (boolean? false)
					   (= suggestion made-suggestion))
				      ;; (if x y #t) -> (or (not x) y)
				      (lint-format "perhaps ~A" caller
						   (let ((nexpr (if false 
								    (if (and (pair? expr) (eq? (car expr) 'not))
									`(or ,(cadr expr) ,true) 
									`(or (not ,expr) ,true))
								    `(and ,expr ,true))))
						     (lists->string form (simplify-boolean nexpr () () env)))))))
			     ((= len 4)
			      ;; (if x (+ y 1) (+ y 1)) -> (+ y 1)
			      (lint-format "if is not needed here: ~A" caller 
					   (lists->string form (if (not (side-effect? test env))
								   true
								   `(begin ,expr ,true))))))
		       
		       (when (and (= suggestion made-suggestion)
				  (not (equal? expr test))) ; make sure the boolean simplification gets reported
			 ;; (or (not (pair? x)) (not (pair? z))) -> (not (and (pair? x) (pair? z)))
			 (lint-format "perhaps ~A" caller (lists->string test expr)))
		       
		       (when (pair? true)
			 (if (and (pair? test)
				  (pair? (cdr true))
				  (null? (cddr true))
				  (or (equal? test (cadr true))
				      (equal? expr (cadr true))))
			     (lint-format "perhaps ~A" caller
					  (lists->string form 
							 (if (eq? false 'no-false)
							     `(cond (,expr => ,true-op))
							     `(cond (,expr => ,true-op) (else ,false))))))
			 
			 (when (and (pair? false)
				    (eq? true-op 'if)
				    (eq? false-op 'if)
				    (= (length true) (length false) 4)
				    (equal? (cadr true) (cadr false)))
			   (if (and (equal? (caddr true) (cadddr false)) ; (if A (if B a b) (if B b a)) -> (if (eq? (not A) (not B)) a b) 
				    (equal? (cadddr true) (caddr false)))
			       (let* ((switch #f)
				      (a (if (and (pair? expr)
						  (eq? (car expr) 'not))
					     (begin (set! switch #t) expr)
					     (simplify-boolean `(not ,expr) () () env)))
				      (b (if (and (pair? (cadr true))
						  (eq? (caadr true) 'not))
					     (begin (set! switch (not switch)) (cadr true))
					     (simplify-boolean `(not ,(cadr true)) () () env))))
				 (lint-format "perhaps ~A" caller
					      (lists->string form 
							     (if switch
								 `(if (eq? ,a ,b) ,(caddr false) ,(caddr true))
								 `(if (eq? ,a ,b) ,(caddr true) ,(caddr false))))))
			       (unless (or (side-effect? expr env)
					   (equal? (cddr true) (cddr false))) ; handled elsewhere
				 (if (equal? (caddr true) (caddr false))  ; (if A (if B a b) (if B a c)) -> (if B a (if A b c))
				     (lint-format "perhaps ~A" caller
						  (lists->string form
								 `(if ,(cadr true) ,(caddr true) 
								      (if ,expr ,(cadddr true) ,(cadddr false)))))
				     (if (equal? (cadddr true) (cadddr false)) ; (if A (if B a b) (if B c b)) -> (if B (if A a c) b)
					 (lint-format "perhaps ~A" caller
						      (lists->string form
								     `(if ,(cadr true)
									  (if ,expr ,(caddr true) ,(caddr false))
									  ,(cadddr true))))))))))
		       ;; --------
		       (when (and (= suggestion made-suggestion)
				  (not (= line-number last-if-line-number)))
			 ;; unravel complicated if-then-else nestings into a single cond, if possible.
			 ;;
			 ;; The (> new-len *report-nested-if*) below can mean (nearly) all nested ifs are turned into conds.
			 ;;   For a long time I thought the if form was easier to read, but now
			 ;;   I like cond better.  But cond also has serious readability issues:
			 ;;   it needs to be clearer where the test separates from the result,
			 ;;   and in a stack of these clauses, it's hard to see anything at all.
			 ;;   Maybe a different color for the test than the result?
			 ;;
			 ;; Also, the check for tree-leaves being hugely different is taken
			 ;;   from C -- I think it is easier to read a large if statement if
			 ;;   the shortest clause is at the start -- especially in a nested if where
			 ;;   it can be nearly impossible to see which dangling one-liner matches
			 ;;   which if (this even in emacs because it unmarks or doesn't remark the matching
			 ;;   paren as you're trying to scroll up to it). 
			 ;;
			 ;; the cond form is not always an improvement:
			 ;;   (if A (if B (if C a b) (if C c d)) (if B (if C e f) (if C g h)))
			 ;;   (cond (A (cond (B (cond (C a) (else b))) ... oh forget it ...))))
			 ;;   perhaps: (case (+ (if A 4 0) (if B 2 0) (if C 1 0)) ((#b000)...))!
			 ;;   how often (and how deeply nested) does this happen? -- not very, but nesting can be ridiculous.
			 ;;   and this assumes all tests are always hit
			 
			 (define (swap-clauses form)
			   (if (not (pair? (cdddr form)))
			       form
			       (let ((expr (cadr form))
				     (ltrue (caddr form))
				     (lfalse (cadddr form)))
				 
				 (let ((true-n (tree-leaves ltrue))
				       (false-n (if (not (pair? lfalse)) 
						    1
						    (tree-leaves lfalse))))
				   
				   (if (< false-n (/ true-n 4))
				       (let ((new-expr (simplify-boolean `(not ,expr) () () env)))
					 (if (and (pair? ltrue)
						  (eq? (car ltrue) 'if))
					     (set! ltrue (swap-clauses ltrue)))
					 (if (and (pair? ltrue)
						  (eq? (car ltrue) 'cond))
					     `(cond (,new-expr ,@(unbegin lfalse))
						    ,@(cdr ltrue))
					     `(cond (,new-expr ,@(unbegin lfalse))
						    (else ,@(unbegin ltrue)))))
				       (begin
					 (if (and (pair? lfalse)
						  (eq? (car lfalse) 'if))
					     (set! lfalse (swap-clauses lfalse)))
					 
					 (if (and (pair? lfalse)
						  (eq? (car lfalse) 'cond))
					     `(cond (,expr ,@(unbegin ltrue))
						    ,@(cdr lfalse))
					     `(cond (,expr ,@(unbegin ltrue))
						    (else ,@(unbegin lfalse))))))))))
			 
			 (let ((new-if (swap-clauses form)))
			   (if (eq? (car new-if) 'cond)
			       (if (> (length new-if) *report-nested-if*)
				   (begin
				     (set! last-if-line-number line-number)
				     (lint-format "perhaps ~A" caller (lists->string form new-if)))
				   
				   (when (= len 4)
				     (let ((true-len (tree-leaves (caddr form))))
				       (if (and (> true-len *report-short-branch*)
						(< (tree-leaves (cadddr form)) (/ true-len *report-short-branch*)))
					   (let ((new-expr (simplify-boolean `(not ,(cadr form)) () () env)))
					     (lint-format "perhaps place the much shorter branch first~A: ~A" caller
							  (local-line-number (cadr form))
							  (truncated-lists->string form `(if ,new-expr ,false ,true))))))))))
			 ;; --------
			 
			 (when (= len 4)
			   ;; move repeated test to top, if no inner false branches
			   ;;   (if A (if B C) (if B D)) -> (if B (if A C D))
			   (when (and (pair? true)         
				      (pair? false)
				      (eq? true-op 'if)
				      (eq? false-op 'if)
				      (equal? (cadr true) (cadr false))
				      (null? (cdddr true))
				      (null? (cdddr false)))
			     (lint-format "perhaps ~A" caller
					  (lists->string form `(if ,(cadr (caddr form))
								   (if ,expr
								       ,(caddr (caddr form))
								       ,(caddr (cadddr form)))))))
			   
			   ;; move repeated start/end statements out of the if
			   (let ((ltrue (if (and (pair? true) (eq? true-op 'begin)) true (list 'begin true)))
				 (lfalse (if (and (pair? false) (eq? false-op 'begin)) false (list 'begin false))))
			     (let ((true-len (length ltrue))
				   (false-len (length lfalse)))
			       (let ((start (if (and (equal? (cadr ltrue) (cadr lfalse))
						     (not (side-effect? expr env))) ; expr might affect start, so we can't pull it ahead
						(list (cadr ltrue))
						()))
				     (end (if (and (not (= true-len false-len 2))
						   (equal? (list-ref ltrue (- true-len 1))
							   (list-ref lfalse (- false-len 1))))
					      (list (list-ref ltrue (- true-len 1)))
					      ())))
				 (when (or (pair? start)
					   (pair? end))
				   (let ((new-true (cdr ltrue))
					 (new-false (cdr lfalse)))
				     (when (pair? end)
				       (set! new-true (copy new-true (make-list (- true-len 2)))) ; (copy lst ()) -> () 
				       (set! new-false (copy new-false (make-list (- false-len 2)))))
				     (when (pair? start)
				       (if (pair? new-true) (set! new-true (cdr new-true)))
				       (if (pair? new-false) (set! new-false (cdr new-false))))
				     (when (or (pair? end)
					       (and (pair? new-true)
						    (pair? new-false))) ; otherwise the rewrite changes the returned value
				       (if (pair? new-true)
					   (set! new-true (if (null? (cdr new-true)) 
							      (car new-true)
							      (cons 'begin new-true))))
				       (if (pair? new-false)
					   (set! new-false (if (null? (cdr new-false)) 
							       (car new-false)
							       (cons 'begin new-false))))
				       ;; (if x (display y) (begin (set! z y) (display y))) -> (begin (if (not x) (set! z y)) (display y))
				       (lint-format "perhaps ~A" caller
						    (lists->string form 
								   (let ((body (if (null? new-true)
										   `(if (not ,expr) ,new-false)
										   (if (null? new-false)
										       `(if ,expr ,new-true)
										       `(if ,expr ,new-true ,new-false)))))
								     `(begin ,@start
									     ,body
									     ,@end))))))))))
			   
			   (when (and (= suggestion made-suggestion) ; (if (not a) A B) -> (if a B A)
				      (not (= line-number last-if-line-number))
				      (pair? expr)
				      (eq? (car expr) 'not)
				      (> (tree-leaves true) (tree-leaves false)))
			     (lint-format "perhaps ~A" caller
					  (lists->string form `(if ,(cadr expr) ,false ,true))))

			   ;; this happens occasionally -- scarcely worth this much code! (gather copied vars outside the if)
			   (when (and (pair? true)
				      (pair? false)
				      (eq? true-op 'let)
				      (eq? false-op 'let)
				      (pair? (cadr true))
				      (pair? (cadr false)))
			     (let ((true-vars (map car (cadr true)))
				   (false-vars (map car (cadr false)))
				   (shared-vars ()))
			       (for-each (lambda (v)
					   (if (and (memq v false-vars)
						    (equal? (cadr (assq v (cadr true)))
							    (cadr (assq v (cadr false)))))
					       (set! shared-vars (cons v shared-vars))))
					 true-vars)
			       (when (pair? shared-vars)
				 ;; now remake true/false lets (maybe nil) without shared-vars
				 (let ((ntv ())
				       (nfv ())
				       (sv ()))
				   (for-each (lambda (v)
					       (if (memq (car v) shared-vars)
						   (set! sv (cons v sv))
						   (set! ntv (cons v ntv))))
					     (cadr true))
				   (set! ntv (if (or (pair? ntv)
						     (pair? (cdddr true))) ; even define is safe here because outer let blocks it just as inner let used to
						 `(let ,(reverse ntv) ,@(cddr true))
						 (caddr true)))
				   (for-each (lambda (v)
					       (if (not (memq (car v) shared-vars))
						   (set! nfv (cons v nfv))))
					     (cadr false))
				   (set! nfv (if (or (pair? nfv)
						     (pair? (cdddr false)))
						 `(let ,(reverse nfv) ,@(cddr false))
						 (caddr false)))
				   ;; (if (> (+ a b) 3) (let ((a x) (c y)) (* a (log c))) (let ((b z) (c y)) (+... ->
				   ;;    (let ((c y)) (if (> (+ a b) 3) (let ((a x)) (* a (log c))) (let ((b z)) (+ b (log c)))))
				   (lint-format "perhaps ~A" caller
						(lists->string form 
							       (if (not (or (side-effect? expr env)
									    (tree-set-member (map car sv) expr)))
								   `(let ,(reverse sv) (if ,expr ,ntv ,nfv))
								   (let ((uniq (find-unique-name form)))
								     `(let ((,uniq ,expr))
									(let ,(reverse sv)
									  (if ,uniq ,ntv ,nfv))))))))))))) 

		       (when (and *report-one-armed-if*
				  (eq? false 'no-false)
				  (or (not (integer? *report-one-armed-if*))
				      (> (tree-leaves true) *report-one-armed-if*)))
			 ;; (if a (begin (set! x y) z)) -> (when a (set! x y) z)
			 (lint-format "~A~A~A perhaps ~A" caller
				      (if (integer? *report-one-armed-if*)
					  "this one-armed if is too big"
					  "")
				      (local-line-number test)
				      (if (integer? *report-one-armed-if*) ";" "")
				      (truncated-lists->string 
				       form (if (and (pair? expr)
						     (eq? (car expr) 'not))
						`(unless ,(cadr expr) ,@(unbegin true))
						`(when ,expr ,@(unbegin true))))))
		       
		       (if (symbol? expr)
			   (set-ref expr caller form env)
			   (lint-walk caller expr env))
		       (set! env (lint-walk caller true env))
		       (if (= len 4) 
			   (set! env (lint-walk caller false env))))))
	     env))
	 (hash-table-set! h 'if if-walker))

	
	;; -------- when, unless --------
	(let ()
	  (define (when-walker caller form env)
	    (if (< (length form) 3)
		(begin
		  (lint-format "~A is messed up: ~A" caller (car form) (truncated-list->string form))
		  env)
		(let ((test (cadr form))
		      (head (car form)))
		  (if (and (pair? test)
			   (eq? (car test) 'not))
		      ;; (when (not a) (set! x y)) -> (unless a (set! x y))
		      (lint-format "perhaps ~A"
				   caller 
				   (truncated-lists->string form
							    `(,(if (eq? head 'when) 'unless 'when)
							      ,(cadr test)
							      ,@(cddr form)))))
		  (if (never-false test)
		      (lint-format "~A test is never false: ~A" caller head (truncated-list->string form))
		      (if (never-true test)       ; (unless #f...)
			  (lint-format "~A test is never true: ~A" caller head (truncated-list->string form))))
		  
		  (if (symbol? test)
		      (begin
			(set-ref test caller form env)
			(if (and (eq? head 'when)
				 (pair? (cddr form))
				 (pair? (caddr form)))
			    (if (memq test (caddr form))
				(and-incomplete form head test (caddr form) env)
				(do ((p (caddr form) (cdr p)))   
				    ((or (not (pair? p))
					 (and (pair? (car p))
					      (memq test (car p))))
				     (if (pair? p)
					 (and-incomplete form head test (car p) env)))))))
		      (when (pair? test)
			(if (and (eq? (car test) 'and)
				 (pair? (cdr test))
				 (pair? (cddr test))
				 (null? (cdddr test)))
			    (let ((arg1 (cadr test))
				  (arg2 (caddr test)))
			      (if (or (and (pair? arg1)
					   (eq? (car arg1) 'not))
				      (and (pair? arg2)
					   (eq? (car arg2) 'not)))
				  (if (eq? head 'unless)
				      ;; (unless (and x (not y)) (display z)) -> (when (or (not x) y) ...)
				      (lint-format "perhaps ~A" caller
						   (lists->string form `(when ,(simplify-boolean `(not ,test) () () env) ...)))
				      (if (and (pair? arg1)
					       (eq? (car arg1) 'not)
					       (pair? arg2)
					       (eq? (car arg2) 'not))
					  ;; (when (and (not x) (not y)) (display z)) -> (unless (or x y) ...)
					  (lint-format "perhaps ~A" caller
						       (lists->string form `(unless (or ,(cadr arg1) ,(cadr arg2)) ...))))))))
			(lint-walk caller test env)))

		  (when (and (pair? (cddr form))           ; (when t1 (if t2 A)) -> (when (and t1 t2) A)
			     (null? (cdddr form))
			     (pair? (caddr form)))
		    (let ((body (caddr form)))
		      (if (eq? (car body) 'cond)           ; (when (cond ...)) -> (cond ...)
			  (lint-format "perhaps ~A" caller
				       (truncated-lists->string form
								`(cond (,(if (eq? (car form) 'when)
									     (simplify-boolean `(not ,(cadr form)) () () env)
									     (cadr form))
									#f)
								       ,@(cdr body))))
			  (when (or (memq (car body) '(when unless))
				    (and (eq? (car body) 'if)
					 (pair? (cdr body))
					 (pair? (cddr body))
					 (null? (cdddr body))))
			    (let ((new-test (let ((inner-test (if (eq? (car body) 'unless)
								  `(not ,(cadr body))
								  (cadr body)))
						  (outer-test (if (eq? head 'unless)
								  `(not ,test)
								  test)))
					      (simplify-boolean `(and ,outer-test ,inner-test) () () env))))
			      ;; (when (and (< x 1) y) (if z (display z))) -> (when (and (< x 1) y z) (display z))
			      (lint-format "perhaps ~A" caller
					   (lists->string form
							  (if (and (pair? new-test)
								   (eq? (car new-test) 'not))
							      `(unless ,(cadr new-test) ,@(cddr body))
							      `(when ,new-test ,@(cddr body))))))))))
		  (lint-walk-open-body caller head (cddr form) env))))
	  (hash-table-set! h 'when when-walker)
	  (hash-table-set! h 'unless when-walker))
	
	
	;; ---------------- cond ----------------
	(let ()
	 (define (cond-walker caller form env)
	   (let ((ctr 0)
		 (suggest made-suggestion)
		 (len (- (length form) 1)))
	     (if (< len 1)
		 (lint-format "cond is messed up: ~A" caller (truncated-list->string form))
		 (let ((exprs ())
		       (result :unset)
		       (has-else #f)
		       (has-combinations #f)
		       (simplifications ())
		       (prev-clause #f)
		       (all-eqv #t)
		       (eqv-select #f))

		   ;; (cond (A (and B C)) (else (and B D))) et al never happens
		   ;;    also (cond (A C) (B C)) -> (if (or A B) C) [non-pair C]
		   ;; ----------------
		   ;; if regular cond + else
		   ;;   scan all return blocks
		   ;;   if all one form, and either header or trailer always match,
		   ;;   rewrite as header + cond|if + trailer
		   ;; given values and the presence of else, every possibility is covered
		   ;; at least (car result) has to match across all
		   (when (and (> len 1) ; (cond (else ...)) is handled elsewhere
			      (pair? (cdr form))
			      (pair? (cadr form))
			      (not (tree-set-member '(unquote #_{list}) form)))
		     (let ((first-clause (cadr form))
			   (else-clause (list-ref form len)))
		       (when (and (pair? (cdr first-clause))
				  (null? (cddr first-clause))
				  (pair? (cadr first-clause))
				  (pair? else-clause))
			 (let ((first-result (cadr first-clause))
			       (first-func (caadr first-clause)))
			   (if (and (memq (car else-clause) '(#t else))
				    (pair? (cdr else-clause))
				    (pair? (cadr else-clause))
				    (or (equal? (caadr first-clause) (caadr else-clause)) ; there's some hope we'll match
					(escape? (cadr else-clause) env)))
			       (let ((else-error (escape? (cadr else-clause) env)))
				 (when (and (pair? (cdr first-result))
					    (not (eq? first-func 'values))
					    (or (not (hash-table-ref syntaces first-func))
						(eq? first-func 'set!))
					    (every? (lambda (c)
						      (and (pair? c)
							   (pair? (cdr c))
							   (pair? (cadr c))
							   (null? (cddr c))
							   (pair? (cdadr c))
							   (or (equal? first-func (caadr c))
							       (and (eq? c else-clause)
								    else-error))))
						    (cddr form)))
				   ((lambda (header-len trailer-len result-min-len)
				      (when (and (or (not (eq? first-func 'set!))
						     (> header-len 1))
						 (or (not (eq? first-func '/))
						     (> header-len 1)
						     (> trailer-len 0)))
					(let ((header (copy first-result (make-list header-len)))
					      (trailer (copy first-result (make-list trailer-len) (- (length first-result) trailer-len))))
					  (if (= len 2)
					      (unless (equal? first-result (cadr else-clause)) ; handled elsewhere (all results equal -> result)
						;; (cond (x (for-each (lambda (x) (display (+ x a))) (f y))) (else (for-each... ->
						;;    (for-each (lambda (x) (display (+ x a))) (if x (f y) (g y)))
						(lint-format "perhaps ~A" caller
							     (let ((else-result (cadr else-clause)))
							       (let ((first-mid-len (- (length first-result) header-len trailer-len))
								     (else-mid-len (- (length else-result) header-len trailer-len)))
								 (let ((fmid (if (= first-mid-len 1)
										 (list-ref first-result header-len)
										 `(values ,@(copy first-result (make-list first-mid-len) header-len))))
								       (emid (if else-error
										 else-result
										 (if (= else-mid-len 1)
										     (list-ref else-result header-len)
										     `(values ,@(copy else-result (make-list else-mid-len) header-len))))))
								   (lists->string form `(,@header (if ,(car first-clause) ,fmid ,emid) ,@trailer)))))))
					      ;; len > 2 so use cond in the revision
					      (let ((middle (map (lambda (c)
								   (let ((test (car c))
									 (result (cadr c)))
								     (let ((mid-len (- (length result) header-len trailer-len)))
								       (if (and else-error 
										(eq? c else-clause))
									   else-clause
									   `(,test ,(if (= mid-len 1)
											(list-ref result header-len)
											`(values ,@(copy result (make-list mid-len) header-len))))))))
								 (cdr form))))
						;; (cond ((< x 1) (+ x 1)) ((< y 1) (+ x 3)) (else (+ x 2))) -> (+ x (cond ((< x 1) 1) ((< y 1) 3) (else 2)))
						(lint-format "perhaps ~A" caller
							     (lists->string form `(,@header (cond ,@middle) ,@trailer))))))))
				    (partition-form (cdr form) (if else-error (- len 1) len)))))
			       
			       ;; not escaping else here because the trailing args might be evaluated first
			       (when (and (not (hash-table-ref syntaces (car first-result)))
					  (every? (lambda (c)
						    (and (pair? c)
							 (pair? (cdr c))
							 (pair? (cadr c))
							 (null? (cddr c))
							 (not (hash-table-ref syntaces (caadr c)))
							 (equal? (cdadr c) (cdr first-result))))
						  (cddr form)))
				 (if (every? (lambda (c)
					       (eq? first-func (caadr c))) ; all result clauses are the same!?
					     (cddr form))                          ; possibly no else, so not always a duplicate message
				     ;; (cond (X (f y z)) (Y (f y z)) (Z (f y z))) -> (if (or X Y Z) (f y z))
				     (lint-format "perhaps ~A" caller
						  (lists->string form
								 `(if (or ,@(map car (cdr form)))
								      ,first-result)))
				     ;; here we need an else clause else (apply #<unspecified> args)
				     (if (memq (car else-clause) '(#t else))
					 ;; (cond (X (f y z)) (else (g y z))) -> ((cond (X f) (else g)) y z)
					 (lint-format "perhaps ~A" caller
						      (lists->string form
								     `((cond ,@(map (lambda (c)
										      (list (car c) (caadr c)))
										    (cdr form)))
								       ,@(cdr first-result))))))))))))
		   ;; ----------------
		   (let ((falses ())
			 (trues ()))
		     (for-each
		      (lambda (clause)
			(set! ctr (+ ctr 1))
			(if (not (pair? clause))
			    (begin
			      (set! all-eqv #f)
			      (set! has-combinations #f)
			      ;; ; (cond 1)
			      (lint-format "cond clause ~A in ~A is not a pair?" caller clause (truncated-list->string form))) 
			    (begin
			      
			      (when all-eqv
				(unless eqv-select
				  (set! eqv-select (eqv-selector (car clause))))
				(set! all-eqv (and eqv-select
						   (not (and (pair? (cdr clause))
							     (eq? (cadr clause) '=>))) ; case sends selector, but cond sends test result
						   (cond-eqv? (car clause) eqv-select #t))))
			      
			      (if (and (pair? prev-clause)
				       (not has-combinations)
				       (> len 2) 
				       (equal? (cdr clause) (cdr prev-clause)))
				  (if (memq (car clause) '(else #t))        ; (cond ... (x z) (else z)) -> (cond ... (else z))
				      (unless (side-effect? (car prev-clause) env)
					;; (cond (x y) (z 32) (else 32))
					(lint-format* caller
						      "this clause could be omitted: "
						      (truncated-list->string prev-clause)))
				      (set! has-combinations #t)))          ; handle these later
			      (set! prev-clause clause)

			      (let ((expr (simplify-boolean (car clause) trues falses env))
				    (test (car clause))
				    (sequel (cdr clause))
				    (first-sequel (and (pair? (cdr clause)) (cadr clause))))

				(if (not (equal? expr test))
				    (set! simplifications (cons (cons clause expr) simplifications)))

				(if (symbol? test)
				    (if (and (not (eq? test 'else))
					     (pair? first-sequel))
					(if (memq test first-sequel)
					    (and-incomplete form 'cond test first-sequel env)
					    (do ((p first-sequel (cdr p)))   
						((or (not (pair? p))
						     (and (pair? (car p))
							  (memq test (car p))))
						 (if (pair? p)
						     (and-incomplete form 'cond test (car p) env))))))
				    (if (and (pair? test)
					     (pair? first-sequel)
					     (hash-table-ref bools (car test)))
					(if (member (cadr test) first-sequel)
					    (and-forgetful form 'cond test first-sequel env)
					    (do ((p first-sequel (cdr p)))   
						((or (not (pair? p))
						     (and (pair? (car p))
							  (member (cadr test) (car p))))
						 (if (pair? p)
						     (and-forgetful form 'cond test (car p) env)))))))
				;; code here to check every arg against its use in the sequel found no problems?!?

				(cond ((memq test '(else #t))
				       (set! has-else #t)
				       
				       (when (pair? sequel)
					 (if (eq? first-sequel #<unspecified>)
					     ;; (cond ((= x y) z) (else #<unspecified>)
					     (lint-format "this #<unspecified> is redundant: ~A" caller clause))
				       
					 (if (and (pair? first-sequel)   ; (cond (a A) (else (cond ...))) -> (cond (a A) ...)
						  (null? (cdr sequel)))  ;    similarly for if, when, and unless
					     (case (car first-sequel)
					       ((cond)
						;; (cond ((< x 1) 2) (else (cond ((< y 3) 2) (#t 4))))
						(lint-format "else clause could be folded into the outer cond: ~A" caller 
							     (lists->string form (append (copy form (make-list ctr)) 
											 (cdr first-sequel)))))
					       ((if)
						;; (cond (a A) (else (if b B)))
						(lint-format "else clause could be folded into the outer cond: ~A" caller 
							     (lists->string form 
									    (append (copy form (make-list ctr)) 
										    (if (= (length first-sequel) 3)
											(list (cdr first-sequel))
											`((,(cadr first-sequel) ,@(unbegin (caddr first-sequel)))
											  (else ,@(unbegin (cadddr first-sequel)))))))))
					       ((when unless)
						;; (cond (a A) (else (when b B)))
						(lint-format "else clause could be folded into the outer cond: ~A" caller 
							     (lists->string form 
									    (append (copy form (make-list ctr))
										    (if (eq? (car first-sequel) 'when)
											`((,(cadr first-sequel) ,@(cddr first-sequel)))
											`(((not ,(cadr first-sequel)) ,@(cddr first-sequel))))))))))))
				      ((not (= ctr len)))

				      ((equal? test ''else)
				       ;; (cond (x y) ('else z))
				       (lint-format "odd cond clause test: is 'else supposed to be else? ~A" caller
						    (truncated-list->string clause)))
				      
				      ((and (eq? test 't)
					    (not (var-member 't env)))
				       ;; (cond ((= x 1) 1) (t 2)
				       (lint-format "odd cond clause test: is t supposed to be #t? ~A" caller
						    (truncated-list->string clause))))
				
				(if (never-false expr)
				    (if (not (= ctr len))
					;; (cond ((getenv s) x) ((= y z) w))
					(lint-format "cond test ~A is never false: ~A" caller (car clause) (truncated-list->string form))
					(if (not (or (memq expr '(#t else))
						     (side-effect? test env)))
					    (lint-format "cond last test could be #t: ~A" caller form)))
				    (if (never-true expr)
					;; (cond ((< 3 1) 2))
					(lint-format "cond test ~A is never true: ~A" caller (car clause) (truncated-list->string form))))
				
				(unless (side-effect? test env)
				  (if (and (not (memq test '(else #t)))
					   (pair? sequel)
					   (null? (cdr sequel)))
				      (cond ((equal? test first-sequel)
					     ;; (cond ((= x 0) x) ((= x 1) (= x 1)))
					     (lint-format "no need to repeat the test: ~A" caller (lists->string clause (list test))))
					    
					    ((and (pair? first-sequel)
						  (pair? (cdr first-sequel))
						  (null? (cddr first-sequel))
						  (equal? test (cadr first-sequel)))
					     (if (eq? (car first-sequel) 'not)
						 ;; (cond ((> x 2) (not (> x 2))))
						 (lint-format "perhaps replace ~A with #f" caller first-sequel)
						 ;; (cond (x (abs x)))
						 (lint-format "perhaps use => here: ~A" caller 
							      (lists->string clause (list test '=> (car first-sequel))))))
					    
					    ((and (eq? first-sequel #t)
						  (pair? test)
						  (not (memq (car test) '(or and)))
						  (eq? (return-type (car test) env) 'boolean?))
					     ;; (cond ((null? x) #t) (else y))
					     (lint-format "this #t could be omitted: ~A" caller (truncated-list->string clause)))))
				  (if (member test exprs)
				      ;; (cond ((< x 2) 3) ((> x 0) 4) ((< x 2) 5))
				      (lint-format "cond test repeated: ~A" caller (truncated-list->string clause))
				      (set! exprs (cons test exprs))))
				
				(if (boolean? expr)
				    (if (not expr)
					;; (cond ((< 3 1) 2))
					(lint-format "cond test is always false: ~A" caller (truncated-list->string clause))
					(if (not (= ctr len))
					    ;; (cond (#t 2) (x 3))
					    (lint-format "cond #t clause is not the last: ~A" caller (truncated-list->string form))))
				    (if (eq? test 'else)
					(if (not (= ctr len))
					    ;; (cond (else 2) (x 3))
					    (lint-format "cond else clause is not the last: ~A" caller (truncated-list->string form)))
					(lint-walk caller test env)))
				
				(if (and (symbol? expr)
					 (not (var-member expr env))
					 (procedure? (symbol->value expr *e*)))
				    ;; (cond (< x 1) (else 1))
				    (lint-format "strange cond test: ~A in ~A is a procedure" caller expr clause))
				
				(if (eq? result :unset)
				    (set! result sequel)
				    (if (not (equal? result sequel))
					(set! result :unequal)))
				
				(cond ((not (pair? sequel))
				       (if (not (null? sequel))  ; (not (null?...)) here is correct -- we're looking for stray dots
					   (lint-format "cond clause is messed up: ~A" caller (truncated-list->string clause))))
				      
				      ((not (eq? first-sequel '=>))
				       (lint-walk-open-body caller 'cond sequel env))
				      
				      ((or (not (pair? (cdr sequel)))
					   (pair? (cddr sequel)))
				       ;; (cond (x =>))
				       (lint-format "cond => target is messed up: ~A" caller (truncated-list->string clause)))
				      
				      (else (let ((f (cadr sequel)))
					      (if (symbol? f)
						  (let ((val (symbol->value f *e*)))
						    (when (procedure? val)
						      (if (not (aritable? val 1)) ; here values might be in test expr
							  ;; (cond (x => expt))
							  (lint-format "=> target (~A) may be unhappy: ~A" caller f clause))
						      (let ((sig (procedure-signature val)))
							(if (and (pair? sig)
								 (pair? (cdr sig)))
							    (let ((from-type (->lint-type expr))
								  (to-type (cadr sig)))
							      (if (not (or (memq from-type '(#f #t values))
									   (memq to-type '(#f #t values))
									   (any-compatible? to-type from-type)))
								  ;; (cond ((> x 0) => abs) (else y))
								  (lint-format "in ~A, ~A returns a ~A, but ~A expects ~A" caller
									       (truncated-list->string clause)
									       expr (prettify-checker-unq from-type)
									       f to-type)))))))
						  (if (and (pair? f)
							   (eq? (car f) 'lambda)
							   (pair? (cdr f))
							   (pair? (cadr f))
							   (not (= (length (cadr f)) 1)))
						      (lint-format "=> target (~A) may be unhappy: ~A" caller f clause)))
					      (lint-walk caller f env))))
				
				(if (side-effect? expr env)
				    (begin
				      (set! falses ())
				      (set! trues ())
				      (set! result :unequal))
				    (begin
				      (if (not (member expr falses))
					  (set! falses (cons expr falses)))
				      (when (pair? expr)
					(if (and (eq? (car expr) 'not)
						 (not (member (cadr expr) trues)))
					    (set! trues (cons (cadr expr) trues)))
					(if (eq? (car expr) 'or)
					    (for-each (lambda (p) 
							(if (not (member p falses))
							    (set! falses (cons p falses))))
						      (cdr expr))))))))))
		      (cdr form))) ; for-each clause
		   
		   (if has-else 
		       (if (pair? result) ; all result clauses are the same (and not implicit)
			   ;; (cond (x #t) (else #t)) -> #t
			   (lint-format "perhaps ~A" caller (lists->string form 
									   (if (null? (cdr result))
									       (car result)
									       `(begin ,@result)))))
		       (let* ((last-clause (and (> len 1)
						(list-ref form len)))
			      (last-res (let ((clen (and (pair? last-clause)
							 (length last-clause))))
					  (and (integer? clen)
					       (> clen 1) 
					       (list-ref last-clause (- clen 1))))))
			 (if (and (pair? last-res)
				  (memq (car last-res) '(#t else)))
			     ;; (cond (x y) (y z (else 3)))
			     (lint-format "perhaps cond else clause is misplaced: ~A in ~A" caller last-res last-clause))))
		   
		   (when (and (= len 2)
			      (not (check-bool-cond caller form (cadr form) (caddr form) env))
			      (pair? (cadr form))   ; (cond 1 2)!
			      (pair? (caddr form)))
		     (let ((c1 (cadr form))
			   (c2 (caddr form)))
		       (if (equal? (simplify-boolean (car c1) () () env)
				   (simplify-boolean `(not ,(car c2)) () () env))
			   (lint-format "perhaps ~A" caller  ; (cond ((x) y) ((not (x)) z)) -> (cond ((x) y) (else z))
					(lists->string form `(cond ,c1 (else ,@(cdr c2)))))
			   (when (and (pair? (car c1))         ; (cond ((not x) y) (else z)) -> (cond (x z) (else y))
				      (pair? (cdr c1))         ;    null case is handled elsewhere
				      (eq? (caar c1) 'not)
				      (memq (car c2) '(else #t)))
			     (let ((c1-len (tree-leaves (cdr c1))) ; try to avoid the dangling short case as in if
				   (c2-len (tree-leaves (cdr c2))))
			       (when (and (< (+ c1-len c2-len) 100)
					  (> (* c1-len 4) c2-len))   ; maybe 4 is too much
				 (lint-format "perhaps ~A" caller
					      (lists->string form 
							     (if (or (pair? (cddr c1))
								     (pair? (cddr c2)))
								 `(cond (,(cadar c1) ,@(cdr c2)) (else ,@(cdr c1)))
								 `(if ,(cadar c1) ,(cadr c2) ,(cadr c1)))))))))))
		   (when has-combinations
		     (do ((new-clauses ())
			  (current-clauses ())
			  (clauses (cdr form) (cdr clauses)))
			 ((null? clauses)
			  (let ((len2 (= (length new-clauses) 2)))
			    (unless (and len2         ; i.e. don't go to check-bool-cond
					 (check-bool-cond caller form (cadr new-clauses) (car new-clauses) env))
			      ;; (cond ((= x 3) 3) ((= x 2) 4) ((= x 1) 4)) -> (case x ((3) 3) ((2 1) 4))
			      (lint-format "perhaps ~A" caller 
					   (lists->string 
					    form
					    (cond (all-eqv
						   (cond->case eqv-select (reverse new-clauses)))
						  ((not (and len2 
							     (pair? (car new-clauses))
							     (memq (caar new-clauses) '(else #t))
							     (pair? (cadr new-clauses))
							     (pair? (caadr new-clauses))
							     (eq? (caaadr new-clauses) 'or)
							     (null? (cdadr new-clauses))))
						   `(cond ,@(reverse new-clauses)))
						  ((null? (cddar new-clauses))  ; (cond (A) (B) (else C)) -> (or A B C)
						   `(or ,@(cdaadr new-clauses) ,(cadar new-clauses)))
						  (else `(or ,@(cdaadr new-clauses) (begin ,@(cdar new-clauses))))))))
			    (set! simplifications ())
			    (set! all-eqv #f)))
		       
		       (let* ((clause (car clauses))
			      (result (cdr clause))) ; can be null in which case the test is the result
			 (cond ((and (pair? simplifications)
				     (assq clause simplifications))
				=> (lambda (e)
				     (set! clause (cons (cdr e) result)))))
			 (if (and (pair? (cdr clauses))
				  (equal? result (cdadr clauses)))
			     (set! current-clauses (cons clause current-clauses))
			     (if (pair? current-clauses)
				 (begin
				   (set! current-clauses (cons clause current-clauses))
				   (set! new-clauses (cons 
						      (cons (simplify-boolean `(or ,@(map car (reverse current-clauses))) () () env)
							    result)
						      new-clauses))
				   (set! current-clauses ()))
				 (set! new-clauses (cons clause new-clauses)))))))
		   
		   (when (and all-eqv
			      (> len (if has-else 2 1))) ; (cond (x y)) -- kinda dumb, but (if x y) isn't much shorter
		     ;; (cond ((= x 0) x) ((= x 1) (= x 1))) -> (case x ((0) x) ((1) (= x 1)))
		     (lint-format "perhaps use case instead of cond: ~A" caller
				  (lists->string form (cond->case eqv-select (cdr form)))))
		   
		   (if (and (= len 2)
			    has-else
			    (null? (cdadr form)))
		       (let ((else-clause (if (null? (cddr (caddr form)))
					      (cadr (caddr form))
					      `(begin ,@(cdr (caddr form))))))
			 ;; (cond ((a)) (else A)) -> (or (a) A)
			 (lint-format "perhaps ~A" caller (lists->string form `(or ,(caadr form) ,else-clause)))))
		   
		   ;; --------
		   (unless (or has-combinations all-eqv)
		     ;; look for repeated ((op x c1) c2) -> ((assoc x '((c1 . c2)...)) => cdr) anywhere in the clause list
		     (let ((nc ())
			   (op #f)
			   (sym-access #f)
			   (start #f)
			   (changed #f))
		       
		       ;; extending this to memx possibilities got only 1 hit and involved ca. 20 lines
		       
		       (define (car-with-expr cls)
			 (cond ((and (pair? simplifications)
				     (assq cls simplifications))
				=> (lambda (e)
				     (set! changed #t)
				     (cons (cdr e) (cdr cls))))
			       (else cls)))
		       
		       (define (start-search clauses test)
			 (if (code-constant? (cadr test))
			     (if (memq (car test) '(= string=? string-ci=? eq? eqv? equal? char=? char-ci=?)) 
				 (set! sym-access caddr))
			     (if (code-constant? (caddr test))
				 (set! sym-access cadr)))
			 (if sym-access 
			     (begin
			       (set! start clauses)
			       (set! op (car test)))
			     (set! nc (cons (car-with-expr (car clauses)) nc))))
		       
		       (do ((clauses (cdr form) (cdr clauses)))
			   ((or (null? clauses)
				(not (pair? (car clauses))))
			    (if (and changed 
				     (null? clauses))
				;; (cond ((< x 2) 3) ((> x 0) 4) ((< x 2) 5)) -> (cond ((< x 2) 3) ((> x 0) 4))
				(lint-format "perhaps ~A" caller
					     (lists->string form `(cond ,@(reverse (map (lambda (c)
											  (if (not (car c)) (values) c))
											nc)))))))
			 (let ((test (caar clauses)))
			   (let ((ok-but-at-end #f)
				 (looks-ok (let ((result (cdar clauses)))
					     (and (pair? test)
						  (pair? (cdr test))
						  (pair? (cddr test))
						  (null? (cdddr test))
						  (pair? result)
						  (null? (cdr result))
						  (not (symbol? (car result)))
						  (or (not (pair? (car result))) ; quoted lists look bad in this context
						      (and (eq? (caar result) 'quote)
							   (not (pair? (cadar result)))))))))
			     (if (not start)
				 (if (and looks-ok
					  (not (null? (cdr clauses))))
				     (start-search clauses test)
				     (set! nc (cons (car-with-expr (car clauses)) nc)))
				 
				 (unless (and looks-ok
					      (eq? (car test) op)
					      (equal? (sym-access test) (sym-access (caar start)))
					      (code-constant? ((if (eq? sym-access cadr) caddr cadr) test))
					      (not (set! ok-but-at-end (null? (cdr clauses)))))
				   
				   (if (eq? (cdr start) clauses) ; only one item in the block, or two but it's just two at the end
				       (begin
					 (set! nc (cons (car start) nc))
					 (if (and looks-ok
						  (not (null? (cdr clauses))))
					     (start-search clauses test)
					     (begin
					       (set! start #f)
					       (set! nc (cons (car-with-expr (car clauses)) nc)))))
				       
				       ;; multiple hits -- can we combine them?
				       (let ((alist ())
					     (cc (if (eq? sym-access cadr) caddr cadr)))
					 (set! changed #t)
					 (do ((sc start (cdr sc)))
					     ((if ok-but-at-end
						  (null? sc)
						  (eq? sc clauses))
					      (case op
						((eq?)         
						 (set! nc (cons `((assq ,(sym-access (caar start)) ',(reverse alist)) => cdr) nc)))
						
						((eqv? char=?) 
						 (set! nc (cons `((assv ,(sym-access (caar start)) ',(reverse alist)) => cdr) nc)))
						
						((equal?)      
						 (set! nc (cons `((assoc ,(sym-access (caar start)) ',(reverse alist)) => cdr) nc)))
						
						((string=?)
						 ;; this is probably faster than assoc + string=?, but it creates symbols
						 (let ((nlst (map (lambda (c)
								    (cons (string->symbol (car c)) (cdr c)))
								  alist)))
						   (set! nc (cons `((assq (string->symbol ,(sym-access (caar start))) ',(reverse nlst)) => cdr) nc))))
						
						(else          
						 (set! nc (cons `((assoc ,(sym-access (caar start)) ',(reverse alist) ,op) => cdr) nc)))))
					   
					   (set! alist (cons (cons (unquoted (cc (caar sc))) (unquoted (cadar sc))) alist)))
					 
					 (if (and looks-ok
						  (not (null? (cdr clauses))))
					     (start-search clauses test)
					     (begin
					       (set! start #f)
					       (if (not ok-but-at-end)
						   (set! nc (cons (car-with-expr (car clauses)) nc))))))))))))))

		   ;; look for case at end (case in the middle is tricky due to #f handling)
		   (when (and (> len 3)
			      (= suggest made-suggestion))
		     (let ((rform (reverse form))
			   (eqv-select #f)
			   (elen (if has-else (- len 1) len)))
		       (if has-else (set! rform (cdr rform)))
		       (set! eqv-select (eqv-selector (caar rform)))
		       (when eqv-select
			 (do ((clauses rform (cdr clauses))
			      (ctr 0 (+ ctr 1)))
			     ((or (null? clauses)
				  (let ((clause (car clauses)))
				      (or (and (pair? (cdr clause))
					       (eq? (cadr clause) '=>)) ; case sends selector, but cond sends test result
					  (not (cond-eqv? (car clause) eqv-select #t)))))
			      (when (and (pair? clauses)
					 (> ctr 1))
				;; (cond ((pair? x) 3) ((eq? x 'a) z) ((eq? x 'b) (* 2 z)) ((eq? x 'c)... ->
				;;    (if (pair? x) 3 (case x ((a) z) ((b) (* 2 z)) ((c) (display z))))
				(lint-format "possibly use case at the end: ~A" caller
					     (lists->string form
							    (let ((else-case (cond->case eqv-select  ; cond->case will handle the else branch
											 (list-tail (cdr form) (- elen ctr)))))
							      (if (= (- elen ctr) 1)
								  (if (equal? (cdadr form) '(#f))
								      `(and (not ,(caadr form)) ,else-case)
								      `(if ,@(cadr form) ,else-case))
								  `(cond ,@(copy (cdr form) (make-list (- elen ctr)))
									 (else ,else-case))))))))))))
		   ;; --------

		   ;; repeated test exprs handled once
		   (let ((exprs ())
			 (reps ())
			 (ctr 0)
			 (pos 0)
			 (head-len 0)
			 (else-leaves 0)
			 (else-result #f))
		     (for-each (lambda (c)
				 (set! pos (+ pos 1))
				 (cond ((and (pair? c)
					     (memq (car c) '(#t else)))
					(set! else-result (cdr c))
					(set! else-leaves (tree-leaves else-result)))

				       ((not (and (pair? c)
						  (pair? (car c))
						  (or (eq? (caar c) 'and)
						      (member (car c) reps))))
					(set! exprs ())
					(set! reps ())
					(set! ctr 0))

				       ((null? exprs)
					(set! head-len pos)
					(set! exprs (cdar c))
					(set! reps exprs)
					(set! ctr 1))

				       (else
					(set! ctr (+ ctr 1))
					(set! reps (remove-if (lambda (rc)
								(not (or (equal? rc (car c))
									 (member rc (cdar c)))))
							      reps)))))
			       (cdr form))
		     (when (and (pair? reps)
				(> ctr 1)
				(< else-leaves (* ctr (length reps) 3)))
		       ;; (cond ((pair? z) 32) ((and (pair? x) (pair? w)) 12) ((pair? x) 2) (else 0)) ->
		       ;;    (cond ((pair? z) 32) ((not (pair? x)) 0) ((pair? w) 12) (else 2))
		       (lint-format "perhaps ~A" caller
				    (lists->string form
						   (let ((not-reps 
							  (simplify-boolean (if (null? (cdr reps))
										`(not ,(car reps))
										`(not (and ,@reps)))
									    () () env)))
						     `(,@(copy form (make-list head-len))
						       (,not-reps
							,@(or else-result '(#<unspecified>)))
						       ,@(let mapper ((clauses (list-tail form head-len))
								      (lst ()))
							   (if (null? clauses)
							       (reverse lst)
							       (let ((new-clause
								      (let ((c (car clauses)))
									(if (memq (car c) '(else #t))
									    c
									    `(,(if (member (car c) reps)
										   'else
										   (remove-if (lambda (rc)
												(member rc reps))
											      (car c)))
									      ,@(cdr c))))))
								 (if (and (pair? new-clause)
									  (pair? (car new-clause))
									  (eq? (caar new-clause) 'and)
									  (pair? (cdar new-clause))
									  (null? (cddar new-clause)))
								     (set-car! new-clause (cadar new-clause)))
								 (if (memq (car new-clause) '(else #t))
								     (reverse (cons new-clause lst))
								     (mapper (cdr clauses) (cons new-clause lst))))))))))))
		   
		   (when (pair? (cadr form)) 
		     (if (= len 1)
			 (let ((clause (cadr form)))       ; (cond (a)) -> a, (cond (a b)) -> (if a b) etc
			   (if (null? (cdr clause))
			       (lint-format "perhaps ~A" caller (lists->string form (car clause)))
			       (if (and (not (eq? (cadr clause) '=>))
					(or (pair? (cddr clause))
					    (= suggest made-suggestion)))
				   ;; (cond ((= x 1) 32)) -> (if (= x 1) 32)
				   (lint-format "perhaps ~A" caller 
						(lists->string form 
							       (if (null? (cddr clause))
								   `(if ,(car clause) ,(cadr clause))
								   (if (and (pair? (car clause))
									    (eq? (caar clause) 'not))
								       `(unless ,@(cdar clause) ,@(cdr clause))						
								       `(when ,(car clause) ,@(cdr clause)))))))))

			 (when has-else ; len > 1 here
			   (let ((last-clause (list-ref form (- len 1)))) ; not the else branch! -- just before it.

			     (when (and (= suggest made-suggestion) ; look for all results the same
					(pair? (cadr form))
					(pair? (cdadr form)))
			       (let ((result (list-ref (cadr form) (- (length (cadr form)) 1)))
				     (else-clause (cdr (list-ref form len))))
				 (when (every? (lambda (c)
						 (and (pair? c)
						      (pair? (cdr c))
						      (equal? result (list-ref c (- (length c) 1)))))
					       (cddr form))
				   ;;  (cond ((and (display x) x) 32) (#t 32)) -> (begin (and (display x) x) 32)
				   (lint-format "perhaps ~A" caller
						(lists->string form
							       (if (= len 2)       ; one is else -- this case is very common
								   (let* ((c1-len (length (cdr last-clause)))
									  (new-c1 (case c1-len
										    ((1) #f)
										    ((2) (cadr last-clause))
										    (else `(begin ,@(copy (cdr last-clause) (make-list (- c1-len 1)))))))
									  (else-len (length else-clause))
									  (new-else (case else-len
										      ((1) #f)
										      ((2) (car else-clause))
										      (else `(begin ,@(copy else-clause (make-list (- else-len 1))))))))
								     `(begin
									,(if (= c1-len 1)
									     (if new-else
										 `(if (not ,(car last-clause)) ,new-else)
										 (car last-clause))
									     (if (= else-len 1)
										 (if new-c1
										     `(if ,(car last-clause) ,new-c1)
										     (car last-clause))
										 `(if ,(car last-clause) ,new-c1 ,new-else)))
									,result))
								   `(begin          ; this almost never happens
								      (cond ,@(map (lambda (c)
										     (let ((len (length c)))
										       (if (= len 2)
											   (if (or (memq (car c) '(else #t))
												   (not (side-effect? (car c) env)))
											       (values)
											       (car c))
											   (copy c (make-list (- len 1))))))
										   (cdr form)))
								      ,result)))))))
			     ;; a few dozen hits here
			     ;;   the 'case parallel gets 2 hits, complex selectors
			     ;;   len = (- (length form) 1) = number of clauses
			     (when (and (> len 2)
					(or (null? (cdr last-clause))
					    (and (pair? (cdr last-clause))
						 (null? (cddr last-clause))
						 (boolean? (cadr last-clause)))))
			       (let ((else-clause (cdr (list-ref form len)))
				     (next-clause (cdr (list-ref form (- len 2)))))
				 (when (and (pair? else-clause)
					    (null? (cdr else-clause))
					    (boolean? (car else-clause))
					    (not (equal? (cdr last-clause) else-clause))
					    (pair? next-clause)
					    (null? (cdr next-clause))
					    (not (boolean? (car next-clause))))
				   (lint-format "perhaps ~A" caller
						(lists->string form
							       `(,@(copy form (make-list (- len 1)))
								 (else ,(if (car else-clause)
									    `(not ,(car last-clause))
									    (car last-clause)))))))))
			     ;; (cond ((= x y) 2) ((= x 2) #f) (else #t)) -> (cond ((= x y) 2) (else (not (= x 2))))
			     ;; (cond ((= x y) 2) ((= x 2) #t) (else #f)) -> (cond ((= x y) 2) (else (= x 2)))

			     (when (= len 3)
			       (let ((first-clause (cadr form))
				     (else-clause (cdr (list-ref form len))))

				 (when (and (or (null? (cdr first-clause))
						(and (null? (cddr first-clause))
						     (boolean? (cadr first-clause))))
					    (pair? last-clause)
					    (or (null? (cdr last-clause))
						(null? (cddr last-clause))))
				   
				   (if (and (pair? (cdr first-clause))
					    (not (cadr first-clause))            ; (cond (A #f) (B #t) (else C)) -> (and (not A) (or B C))
					    (or (null? (cdr last-clause))
						(eq? (cadr last-clause) #t)))
				       (lint-format "perhaps ~A" caller
						    (lists->string form 
								   (simplify-boolean 
								    `(and (not ,(car first-clause))
									  (or ,(car last-clause) 
									      ,@(if (null? (cdr else-clause))
										    else-clause
										    `(begin ,@else-clause))))
								    () () env)))
				       (if (and (or (null? (cdr first-clause))   ; (cond (A #t) (B C) (else #f)) -> (or A (and B C))
						    (eq? (cadr first-clause) #t))
						(not (car else-clause))
						(null? (cdr else-clause)))
					   (lint-format "perhaps ~A" caller
							(lists->string form 
								       `(or ,(car first-clause)
									    (and ,@last-clause)))))))

				 (when (and (equal? (cdr first-clause) else-clause) ; a = else result
					    (pair? (cdr last-clause))               ; b does exist
					    (not (eq? (cadr last-clause) '=>)))     ; no => in b
				   ;; (cond (A a) (B b) (else a)) -> (if (or A (not B)) a b)
				   (lint-format "perhaps ~A" caller
						(lists->string form
							       (let ((A (car first-clause))
								     (a (cdr first-clause))
								     (B (car last-clause))
								     (b (cdr last-clause)))
								 (let ((nexpr (simplify-boolean `(or ,A (not ,B)) () () env)))
								   (cond ((not (and (null? (cdr a))
										    (null? (cdr b))))
									  `(cond (,nexpr ,@a) (else ,@b)))
									 
									 ((eq? (car a) #t)
									  (if (not (car b))
									      nexpr
									      (simplify-boolean `(or ,nexpr ,(car b)) () () env)))
									 
									 ((car a) ; i.e a is not #f
									  `(if ,nexpr ,(car a) ,(car b)))
									 
									 ((eq? (car b) #t)
									  (simplify-boolean `(not ,nexpr) () () env))
									 
									 (else (simplify-boolean `(and (not ,nexpr) ,(car b)) () () env))))))))))
			     (when (> len 3)
			       ;; this is not ideal
			       (let ((e (list-ref form len))      ; (cond (W X) (A B) (C D) (else B)) -> (cond (W X) ((or A (not C)) B) (else D))
				     (b (list-ref form (- len 1)))
				     (a (list-ref form (- len 2))))
				 (if (and (pair? a)
					  (pair? (cdr a)) ; is (else) a legal cond clause? -- yes, it returns else...
					  (pair? e)
					  (equal? (cdr a) (cdr e))
					  (pair? b)
					  (pair? (cdr b))
					  (not (eq? (cadr b) '=>)))
				     (let ((expr (simplify-boolean `(or ,(car a) (not ,(car b))) () () env)))
				       (lint-format "perhaps ~A" caller
						    (lists->string form `(cond ,(if (> len 4) '... (cadr form))
									       (,expr ,@(cdr a)) 
									       (else ,@(cdr b)))))))))
			     
			     (let ((arg1 (cadr form))
				   (arg2 (caddr form)))
			       (when (and (pair? arg1)
					  (pair? (car arg1))
					  (pair? (cdr arg1))
					  (pair? arg2)
					  (eq? (caar arg1) 'and)
					  (null? (cddr arg1))
					  (pair? (cdr arg2))
					  (null? (cddr arg2))
					  (member (car arg2) (cdar arg1))
					  (= (length (cdar arg1)) 2))
				 ;; (cond ((and A B) c) (B d) (else e)) -> (cond (B (if A c d)) (else e))
				 (lint-format "perhaps ~A" caller
					      (lists->string form
							     `(cond (,(car arg2)
								     (if ,((if (equal? (car arg2) (cadar arg1)) caddar cadar) arg1)
									 ,(cadr arg1)
									 ,(cadr arg2)))
								    ,@(cdddr form))))))

			     (if (and (pair? last-clause)   ; (cond ... ((or ...)) (else ...)) -> (cond ... (else (or ... ...)))
				      (pair? (car last-clause))
				      (null? (cdr last-clause))
				      (eq? (caar last-clause) 'or))
				 (let ((else-clause (let ((e (cdr (list-ref form len))))
						      (if (null? (cdr e))
							  (car e)
							  `(begin ,@e)))))
				   ;; (cond ((A) B) ((or C D)) (else E)) -> (cond ((A) B) (else (or C D E)))
				   (lint-format "perhaps ~A" caller
						(lists->string form
							       `(cond ,@(copy (cdr form) (make-list (- len 2)))
								      (else (or ,@(cdar last-clause) ,else-clause))))))))))
		     
		     (let ((last-clause (list-ref form (if has-else (- len 1) len)))) ; not the else branch! -- just before it.
		       (if (and (pair? last-clause)          ; (cond ... (A (cond ...)) (else B)) -> (cond ... ((not A) B) ...)
				(pair? (cdr last-clause))
				(null? (cddr last-clause))
				(pair? (cadr last-clause))
				(memq (caadr last-clause) '(if cond)))
			   (let ((new-test (simplify-boolean `(not ,(car last-clause)) () () env))
				 (new-result (if has-else 
						 (cdr (list-ref form len))
						 (if (eq? form lint-mid-form) 
						     () 
						     (list #<unspecified>)))))
			     (if (eq? (caadr last-clause) 'cond)
				 ;; (cond (A (cond (B c) (else D))) (else E)) -> (cond ((not A) E) (B c) (else D))
				 (lint-format "perhaps ~A" caller
					      (lists->string form
							     `(cond ,@(copy (cdr form) (make-list (- len (if has-else 2 1))))
								    (,new-test ,@new-result)
								    ,@(cdadr last-clause))))
				 (if (= (length (cadr last-clause)) 4)
				     (let ((if-form (cdadr last-clause)))
				       ;; (cond (A B) (C (if D d E)) (else F)) -> (cond (A B) ((not C) F) (D d) (else E))
				       (lint-format "perhaps ~A" caller
						    (lists->string form
								   `(cond ,@(copy (cdr form) (make-list (- len (if has-else 2 1))))
									  (,new-test ,@new-result)
									  (,(car if-form) ,@(unbegin (cadr if-form)))
									  (else ,@(unbegin (caddr if-form))))))))))
			   (when (> len 2)                           ; rewrite nested conds as one cond
			     (let ((lim (if has-else (- len 2) len))
				   (tlen (tree-leaves form)))
			       (when (< tlen 200)
				 (set! tlen (/ tlen 4))
				 (do ((i 0 (+ i 1))
				      (k (+ lim 1) (- k 1))
				      (p (cdr form) (cdr p)))
				     ((or (not (pair? p))
					  (= i lim)))
				   (let ((nc (car p)))
				     (if (and (pair? nc)        
					      (pair? (cdr nc))
					      (null? (cddr nc))
					      (pair? (cadr nc))
					      (eq? (caadr nc) 'cond)
					      (>= (length (cdadr nc)) (* 2 k))
					      (> (tree-leaves nc) tlen))
					 (let ((new-test (simplify-boolean `(not ,(car nc)) () () env))
					       (new-result (if (and has-else
								    (= i (- lim 1))
								    (null? (cddadr p))
								    (null? (cddr (caddr p))))
							       `(if ,(caadr p) ,(cadadr p) ,(cadr (caddr p)))
							       `(cond ,@(cdr p)))))
					   ;; (cond ((= x 0) 1) ((= x 3) (cond ((not y) 1) ((pair? y) 2) ((eq? y 'a) 3) (else 4))) ((< x 200) 2) (else 5)) ->
					   ;;   (cond ((= x 0) 1) ((not (= x 3)) (if (< x 200) 2 5)) ((not y) 1) ((pair? y) 2) ((eq? y 'a) 3) (else 4))
					   (lint-format "perhaps ~A" caller
							(lists->string form
								       `(cond ,@(copy (cdr form) (make-list i))
									      (,new-test ,new-result)
									      ,@(cdadr nc))))))))))))))))
	     env))
	  (hash-table-set! h 'cond cond-walker))
	
	;; ---------------- case ----------------		  
	(let ()
	  (define case-walker 
	    (let ((selector-types '(#t symbol? char? boolean? integer? rational? real? complex? number? null? eof-object?)))
	      (lambda (caller form env)
	    ;; here the keys are not evaluated, so we might have a list like (letrec define ...)
	    ;; also unlike cond, only 'else marks a default branch (not #t)
	    
	    (if (< (length form) 3)
		;; (case 3)
		(lint-format "case is messed up: ~A" caller (truncated-list->string form))
		(let ((sel-type #t)
		      (selector (cadr form))
		      (suggest made-suggestion))
		  
		  ;; ----------------
		  ;; if regular case + else -- just like cond above
		  (let ((len (- (length form) 2))) ; number of clauses
		    (when (and (> len 1)           ; (case x (else ...)) is handled elsewhere
			       (pair? (cdr form))
			       (pair? (cddr form))
			       (pair? (caddr form))
			       (not (tree-set-member '(unquote #_{list}) form)))
		      (let ((first-clause (caddr form))
			    (else-clause (list-ref form (+ len 1))))
			
			(when (and (pair? else-clause)
				   (eq? (car else-clause) 'else)
				   (pair? (cdr first-clause))
				   (pair? (cadr first-clause))
				   (not (hash-table-ref syntaces (caadr first-clause)))
				   (pair? (cdadr first-clause))
				   (null? (cddr first-clause))
				   (every? (lambda (c)
					     (and (pair? c)
						  (pair? (cdr c))
						  (pair? (cadr c))
						  (null? (cddr c))
						  (not (hash-table-ref syntaces (caadr c)))
						  (equal? (cdadr first-clause) (cdadr c))))
					   (cdddr form)))
			  ;; (case x ((a) (f y z)) (else (g y z))) -> ((if (eq? x 'a) f g) y z)
			  (lint-format "perhaps ~A" caller      ; all results share trailing args
				       (lists->string form
						      (if (and (= len 2)
							       (symbol? (caar first-clause))
							       (null? (cdar first-clause)))
							  `((if (eq? ,(cadr form) ',(caar first-clause))
								,(caadr first-clause)
								,(caadr else-clause))
							    ,@(cdadr first-clause))
							  `((case ,(cadr form)
							      ,@(map (lambda (c)
								       (list (car c) (caadr c)))
								     (cddr form)))
							    ,@(cdadr first-clause))))))
			
			(when (and (pair? (cdr first-clause))
				   (null? (cddr first-clause))
				   (pair? (cadr first-clause))
				   (pair? else-clause)
				   (eq? (car else-clause) 'else)
				   (pair? (cdr else-clause))
				   (pair? (cadr else-clause))
				   (or (equal? (caadr first-clause) (caadr else-clause)) ; there's some hope we'll match
				       (escape? (cadr else-clause) env)))
			  (let ((first-result (cadr first-clause))
				(first-func (caadr first-clause))
				(else-error (escape? (cadr else-clause) env)))
			    (when (and (pair? (cdr first-result))
				       (not (eq? first-func 'values))
				       (or (not (hash-table-ref syntaces first-func))
					   (eq? first-func 'set!))
				       (every? (lambda (c)
						 (and (pair? c)
						      (pair? (cdr c))
						      (pair? (cadr c))
						      (null? (cddr c))
						      (pair? (cdadr c))
						      (or (equal? first-func (caadr c))
							  (and (eq? c else-clause)
							       else-error))))
					       (cdddr form)))
			      
			      ((lambda (header-len trailer-len result-mid-len)
				 (when (and (or (not (eq? first-func 'set!))
						(> header-len 1))
					    (or (not (eq? first-func '/))
						(> header-len 1)
						(> trailer-len 0)))
				   (let ((header (copy first-result (make-list header-len)))
					 (trailer (copy first-result (make-list trailer-len) (- (length first-result) trailer-len))))
				     (if (= len 2)
					 (unless (equal? first-result (cadr else-clause)) ; handled elsewhere (all results equal -> result)
					   ;; (case x ((1) (+ x 1)) (else (+ x 3))) -> (+ x (if (eqv? x 1) 1 3))
					   (lint-format "perhaps ~A" caller
							(let ((else-result (cadr else-clause)))
							  (let ((first-mid-len (- (length first-result) header-len trailer-len))
								(else-mid-len (- (length else-result) header-len trailer-len)))
							    (let* ((fmid (if (= first-mid-len 1)
									     (list-ref first-result header-len)
									     `(values ,@(copy first-result (make-list first-mid-len) header-len))))
								   (emid (if else-error
									     else-result
									     (if (= else-mid-len 1)
										 (list-ref else-result header-len)
										 `(values ,@(copy else-result (make-list else-mid-len) header-len)))))
								   (middle (if (= (length (car first-clause)) 1)
									       `(eqv? ,(cadr form) ,(caar first-clause))
									       `(memv ,(cadr form) ',(car first-clause)))))
							      (lists->string form `(,@header (if ,middle ,fmid ,emid) ,@trailer)))))))
					 ;; len > 2 so use case in the revision
					 (let ((middle (map (lambda (c)
							      (let ((test (car c))
								    (result (cadr c)))
								(let ((mid-len (- (length result) header-len trailer-len)))
								  (if (and else-error 
									   (eq? c else-clause))
								      else-clause
								      `(,test ,(if (= mid-len 1)
										   (list-ref result header-len)
										   `(values ,@(copy result (make-list mid-len) header-len))))))))
							    (cddr form))))
					   ;; (case x ((0) (log x 2)) ((1) (log x 3)) (else (error 'oops))) -> (log x (case x ((0) 2) ((1) 3) (else (error 'oops))))
					   (lint-format "perhaps ~A" caller
							(lists->string form `(,@header (case ,(cadr form) ,@middle) ,@trailer))))))))
			       (partition-form (cddr form) (if else-error (- len 1) len)))))))))
		  ;; ----------------

		  (if (every? (lambda (c)               ; (case x ((a) a) ((b) b)) -> (symbol->value x)
				(and (pair? c)
				     (pair? (car c))
				     (symbol? (caar c))
				     (null? (cdar c))
				     (pair? (cdr c))
				     (null? (cddr c))
				     (eq? (caar c) (cadr c)))) ; the quoted case happens only in test suites
			      (cddr form))
		      (lint-format "perhaps (ignoring the unmatched case) ~A" caller (lists->string form `(symbol->value ,(cadr form)))))
		  
		  (when (= suggest made-suggestion)
		    (let ((clauses (cddr form)))            ; (case x ((a) #t) (else #f)) -> (eq? x 'a) -- this stuff actually happens!
		      (if (null? (cdr clauses))
			  (let ((clause (car clauses)))
			    (when (and (pair? clause)
				       (pair? (car clause))
				       (pair? (cdr clause)))
			      ;; (case 3 ((0) #t)) -> (if (eqv? 3 0) #t)
			      ;; (case x ((#(0)) 2)) -> (if (eqv? x #(0)) 2)
			      (lint-format "perhaps ~A" caller
					   (lists->string form
							  (let ((test (cond ((pair? (cdar clause))
									     `(memv ,(cadr form) ',(car clause)))
									    ((and (symbol? (caar clause))
										  (not (keyword? (caar clause))))
									     `(eq? ,(cadr form) ',(caar clause)))
									    ((or (keyword? (caar clause))
										 (null? (caar clause)))
									     `(eq? ,(cadr form) ,(caar clause)))
									    ((not (boolean? (caar clause)))
									     `(eqv? ,(cadr form) ,(caar clause)))
									    ((caar clause)
									     (cadr form))
									    (else `(not ,(cadr form)))))
								(op (if (and (pair? (cdr clause))
									     (pair? (cddr clause)))
									'when 'if)))
							    `(,op ,test ,@(cdr clause)))))))
			  (when (and (null? (cddr clauses))
				     (pair? (car clauses))
				     (pair? (cadr clauses))
				     (eq? (caadr clauses) 'else)
				     (pair? (cdar clauses))
				     (pair? (cdadr clauses))
				     (null? (cddar clauses))
				     (null? (cddadr clauses))
				     (not (equal? (cadadr clauses) (cadar clauses))))
			    (let* ((akey (null? (cdaar clauses)))
				   (keylist ((if akey caaar caar) clauses))
				   (quoted (or (not akey) (symbol? keylist)))
				   (op (if (every? symbol? (caar clauses))
					   (if akey 'eq? 'memq)
					   (if akey 'eqv? 'memv))))
			      ;; can't use '= or 'char=? here because the selector may return anything
			      ;; (case x ((#\a) 3) (else 4)) -> (if (eqv? x #\a) 3 4)
			      ;; (case x ((a) #t) (else #f)) -> (eq? x 'a)
			      (lint-format "perhaps ~A" caller 
					   (lists->string form 
							  (cond ((and (boolean? (cadar clauses))
								      (boolean? (cadadr clauses)))
								 (if (cadadr clauses)
								     (if quoted
									 `(not (,op ,selector ',keylist))
									 `(not (,op ,selector ,keylist)))
								     (if quoted
									 `(,op ,selector ',keylist)
									 `(,op ,selector ,keylist))))

								((not (cadadr clauses)) ; (else #f) happens a few times
								 (simplify-boolean 
								  (if quoted
								      `(and (,op ,selector ',keylist) ,(cadar clauses))
								      `(and (,op ,selector ,keylist) ,(cadar clauses)))
								  () () env))

								(quoted
								 `(if (,op ,selector ',keylist) ,(cadar clauses) ,(cadadr clauses)))

								(else 
								 (let ((select-expr (if (and (eq? op 'eqv?)
											     (boolean? keylist)
											     (or (and (symbol? selector)
												      (not keylist))
												 (and (pair? selector)
												      (symbol? (car selector))
												      (let ((sig (arg-signature (car selector) env)))
													(and (pair? sig)
													     (eq? (car sig) 'boolean?))))))
											(if keylist selector `(not ,selector))
											`(,op ,selector ,keylist))))
								   `(if ,select-expr ,(cadar clauses) ,(cadadr clauses))))))))))))
		  (if (and (not (pair? selector))
			   (constant? selector))
		      ;; (case 3 ((0) #t))
		      (lint-format "case selector is a constant: ~A" caller (truncated-list->string form)))
		  (if (symbol? selector)
		      (set-ref selector caller form env)
		      (lint-walk caller selector env))
		  (if (and (pair? selector)
			   (symbol? (car selector)))
		      (begin
			(set! sel-type (return-type (car selector) env))
			(if (and (symbol? sel-type)
				 (not (memq sel-type selector-types)))
			    ;; (case (list 1) ((0) #t))
			    (lint-format "case selector may not work with eqv: ~A" caller (truncated-list->string selector)))))
		  
		  (let ((all-keys ())
			(all-exprs ())
			(ctr 0)
			(result :unset)
			(exprs-repeated #f)
			(else-foldable #f)
			(has-else #f)
			(len (length (cddr form))))
		    (for-each
		     (lambda (clause)
		       (set! ctr (+ ctr 1))
		       (if (not (pair? clause))
			   (lint-format "case clause should be a list: ~A" caller (truncated-list->string clause))
			   (let ((keys (car clause))
				 (exprs (cdr clause)))
			     (if (null? exprs)
				 ;; (case x (0))
				 (lint-format "clause result is missing: ~A" caller clause))
			     (if (eq? result :unset)
				 (set! result exprs)
				 (if (not (equal? result exprs))
				     (set! result :unequal)))

			     (if (member exprs all-exprs)
				 (set! exprs-repeated exprs)
				 (set! all-exprs (cons exprs all-exprs)))
			     (if (and (pair? exprs)
				      (null? (cdr exprs))
				      (pair? (car exprs))
				      (pair? (cdar exprs))
				      (null? (cddar exprs))
				      (equal? selector (cadar exprs)))
				 (if (and (eq? (caar exprs) 'not)
					  (not (memq #f keys)))
				     ;; (case x ((0) (f x)) ((1) (not x)))
				     (lint-format "in ~A, perhaps replace ~A with #f" caller clause (car exprs))
				     ;; (case x ((0 1) (abs x)))
				     (lint-format "perhaps use => here: ~A" caller 
						  (lists->string clause (list keys '=> (caar exprs))))))

			     (if (pair? keys)
				 (if (not (proper-list? keys))
				     ;; (case x ((0) 1) ((1) 2) ((3 . 0) 4))
				     (lint-format (if (null? keys) 
						      "null case key list: ~A" 
						      "stray dot in case case key list: ~A")
						  caller (truncated-list->string clause))
				     (for-each
				      (lambda (key)
					(if (or (vector? key)
						(string? key)
						(pair? key))
					    ;; (case x ((#(0)) 2))
					    (lint-format "case key ~S in ~S is unlikely to work (case uses eqv? but it is a ~A)" caller
							 key clause
							 (cond ((vector? key) 'vector)
							       ((pair? key) 'pair)
							       (else 'string))))
					(if (member key all-keys)
					    ;; (case x ((0) 1) ((1) 2) ((3 0) 4))
					    (lint-format "repeated case key ~S in ~S" caller key clause)
					    (set! all-keys (cons key all-keys)))
					;; unintentional quote here, as in (case x ('a b)...) never happens and
					;;   is hard to distinguish from (case x ((quote a) b)...) which happens a lot
					(if (not (compatible? sel-type (->lint-type key)))
					    ;; (case (string->symbol x) ((a) 1) ((2 3) 3))
					    (lint-format "case key ~S in ~S is pointless" caller key clause)))
				      keys))
				 (if (not (eq? keys 'else))
				     ;; (case ((1) 1) (t 2))
				     (lint-format "bad case key ~S in ~S" caller keys clause)
				     (begin
				       (set! has-else clause)
				       ;; exprs: (res) or if case, ((case ...)...)
				       (if (not (= ctr len))
					   ;; (case x (else 2) ((0) 1))
					   (lint-format "case else clause is not the last: ~A"
							caller 
							(truncated-list->string (cddr form)))
					   (when (and (pair? exprs)
						      (pair? (car exprs))
						      (null? (cdr exprs)))
					     (case (caar exprs)
					       ((case)                     ; just the case statement in the else clause
						(when (and (equal? selector (cadar exprs))
							   (not (side-effect? selector env)))
						  (set! else-foldable (cddar exprs))))
					       ((if)                       ; just if -- if foldable, make it look like it came from case
						(when (and (equal? selector (eqv-selector (cadar exprs)))
							   (cond-eqv? (cadar exprs) selector #t)
							   (not (side-effect? selector env)))
						  ;; else-foldable as (((keys-from-test) true-branch) (else false-branch))
						  (set! else-foldable 
							(if (pair? (cdddar exprs))
							    `(,(case-branch (cadar exprs) selector (list (caddar exprs)))
							      (else ,(car (cdddar exprs))))
							    (list (case-branch (cadar exprs) selector (cddar exprs)))))))))))))
			     
			     (lint-walk-open-body caller (car form) exprs env))))
		     (cddr form))

		    (if (and has-else
			     (pair? result)
			     (not else-foldable))
			(begin
			  ;; (case x (else (case x (else 1)))) -> 1
			  (lint-format "perhaps ~A" caller (lists->string form 
									  (if (null? (cdr result))
									      (car result)
									      `(begin ,@result))))
			  (set! exprs-repeated #f)))
		    ;; repeated result (but not all completely equal) and with else never happens
		      
		    (when (or exprs-repeated else-foldable)
		      (let ((new-keys-and-exprs ())
			    (mergers ())
			    (else-clause (if else-foldable
					     (call-with-exit
					      (lambda (return)
						(for-each (lambda (c) (if (eq? (car c) 'else) (return c))) else-foldable)
						()))
					     (or has-else ()))))
			
			(let ((merge-case-keys 
			       (let ((else-exprs (and (pair? else-clause) (cdr else-clause))))
				 (define (a-few lst)
				   (if (> (length lst) 3)
				       (copy lst (make-list 4 '...) 0 3)
				       lst))
				 (lambda (clause)
				   (let ((keys (car clause))
					 (exprs (cdr clause)))
				     (when (and (pair? exprs)             ; ignore clauses that are messed up
						(not (eq? keys 'else))
						(not (equal? exprs else-exprs)))
				       (let ((prev (member exprs new-keys-and-exprs (lambda (a b) (equal? a (cdr b))))))
					 (if prev
					     (let* ((cur-clause (car prev))
						    (cur-keys (car cur-clause)))
					       (when (pair? cur-keys)
						 (set! mergers (cons (list (a-few keys) (a-few cur-keys)) mergers))
						 (set-car! cur-clause
							   (append cur-keys
								   (map (lambda (key)
									  (if (memv key cur-keys) (values) key))
									keys)))))
					     (set! new-keys-and-exprs (cons (cons (copy (car clause))
										  (cdr clause))
									    new-keys-and-exprs))))))))))
			  
			  (for-each merge-case-keys (cddr form))
			  (if (pair? else-foldable)
			      (for-each merge-case-keys else-foldable)))
			
			(if (null? new-keys-and-exprs)
			    (lint-format "perhaps ~A" caller 
					 ;; (case x (else (case x (else 1)))) -> 1
					 (lists->string form 
							(if (or (null? else-clause)    ; can this happen? (it's caught above as an error)
								(null? (cdr else-clause)))
							    ()
							    (if (null? (cddr else-clause))
								(cadr else-clause)
								`(begin ,@(cdr else-clause))))))
			    (begin
			      ;; (null? (cdr new-keys-and-exprs)) is rare and kinda dumb -- cases look like test suite entries
			      (for-each 
			       (lambda (clause)
				 (if (and (pair? (car clause))
					  (pair? (cdar clause)))
				     (if (every? integer? (car clause))
					 (set-car! clause (sort! (car clause) <))
					 (if (every? char? (car clause))
					     (set-car! clause (sort! (car clause) char<?))))))
			       new-keys-and-exprs)
			      (let ((new-form (if (pair? else-clause)
						  `(case ,(cadr form) ,@(reverse new-keys-and-exprs) ,else-clause)
						  `(case ,(cadr form) ,@(reverse new-keys-and-exprs)))))
				;; (case x ((0) 32) ((1) 32)) -> (case x ((0 1) 32))
				(lint-format "perhaps ~A" caller
					     (if (pair? mergers)
						 (format #f "merge keys ~{~{~A with ~A~}~^, ~}: ~A" 
							 (reverse mergers) 
							 (lists->string form new-form))
						 (lists->string form new-form)))))))))))
	    env)))
	  (hash-table-set! h 'case case-walker))
	
	
	;; ---------------- do ----------------
	(let ((cxars (hash-table
		      (cons 'car (lambda (sym) sym))
		      (cons 'caar (lambda (sym) (list 'car sym)))
		      (cons 'cdar (lambda (sym) (list 'cdr sym)))
		      (cons 'caaar (lambda (sym) (list 'caar sym)))
		      (cons 'cdaar (lambda (sym) (list 'cdar sym)))
		      (cons 'cddar (lambda (sym) (list 'cddr sym)))
		      (cons 'cadar (lambda (sym) (list 'cadr sym)))
		      (cons 'caaaar (lambda (sym) (list 'caaar sym)))
		      (cons 'caadar (lambda (sym) (list 'caadr sym)))
		      (cons 'cadaar (lambda (sym) (list 'cadar sym)))
		      (cons 'caddar (lambda (sym) (list 'caddr sym)))
		      (cons 'cdaaar (lambda (sym) (list 'cdaar sym)))
		      (cons 'cdadar (lambda (sym) (list 'cdadr sym)))
		      (cons 'cddaar (lambda (sym) (list 'cddar sym)))
		      (cons 'cdddar (lambda (sym) (list 'cdddr sym))))))

	  (define (car-subst sym new-sym tree)
	    (cond ((or (not (pair? tree))
		       (eq? (car tree) 'quote))
		   tree)
		  ((not (and (symbol? (car tree))
			     (pair? (cdr tree))
			     (null? (cddr tree))
			     (eq? sym (cadr tree))))
		   (cons (car-subst sym new-sym (car tree))
			 (car-subst sym new-sym (cdr tree))))

		   ((hash-table-ref cxars (car tree)) => (lambda (f) (f new-sym)))
		   (else tree)))

	  (define (var-step v) ((cdr v) 'step))
		   
	 (define (do-walker caller form env)
	   (let ((vars ()))
	     (if (not (and (>= (length form) 3)
			   (proper-list? (cadr form))
			   (proper-list? (caddr form))))
		 (lint-format "do is messed up: ~A" caller (truncated-list->string form))
		 
		 (let ((step-vars (cadr form))
		       (inner-env #f))

		   ;; do+lambda in body with stepper as free var never happens

		   (if (not (side-effect? form env))
		       (let ((end+result (caddr form)))
			 (if (or (not (pair? end+result))
				 (null? (cdr end+result)))
			     ;; (do ((i 0 (+ i 1))) ((= i 1)))
			     (lint-format "this do-loop could be replaced by (): ~A" caller (truncated-list->string form))
			     (if (and (null? (cddr end+result))
				      (code-constant? (cadr end+result)))
				 ;; (begin (z 1) (do ((i 0 (+ i 1))) ((= i n) 32))): 32
				 (lint-format "this do-loop could be replaced by ~A: ~A" caller (cadr end+result) (truncated-list->string form))))))
		   
		   ;; walk the init forms before adding the step vars to env
		   (do ((bindings step-vars (cdr bindings)))
		       ((not (pair? bindings))
			(if (not (null? bindings))
			    (lint-format "do variable list is not a proper list? ~S" caller step-vars)))
		     (when (binding-ok? caller 'do (car bindings) env #f)
		       (for-each (lambda (v)
				   (if (not (or (eq? (var-initial-value v) (var-name v))
						(not (tree-memq (var-name v) (cadar bindings)))
						(hash-table-ref built-in-functions (var-name v))
						(tree-table-member binders (cadar bindings))))
				       (if (not (var-member (var-name v) env))
					   ;; (let ((xx 0)) (do ((x 1 (+ x 1)) (y x (- y 1))) ((= x 3) xx) (display y)): x
					   (lint-format "~A in ~A does not appear to be defined in the calling environment" caller
							(var-name v) (car bindings))
					   ;; (let ((x 0)) (do ((x 1 (+ x 1)) (y x (- y 1))) ((= x 3)) (display y))): y
					   (lint-format "~A in ~A refers to the caller's ~A, not the do-loop variable" caller
							(var-name v) (car bindings) (var-name v)))))
				 vars)
		       
		       (lint-walk caller (cadar bindings) env)
		       (let ((new-var (let ((v (make-var :name (caar bindings) 
							 :definer 'do
							 :initial-value (cadar bindings))))
					(let ((stepper (and (pair? (cddar bindings)) (caddar bindings))))
					  (varlet (cdr v) :step stepper)
					  (if stepper (set! (var-history v) (cons (list 'set! (caar bindings) stepper) (var-history v)))))
					v)))
			 (set! vars (cons new-var vars)))))
		   
		   (set! inner-env (append vars env))
		   
		   ;; walk the step exprs
		   (let ((baddies ())) ; these are step vars (with step exprs) used within other step vars step expressions			   
		     (do ((bindings step-vars (cdr bindings)))
			 ((not (pair? bindings)))
		       (let ((stepper (car bindings))) ; the entire binding: '(i 0 (+ i 1))
			 (when (and (binding-ok? caller 'do stepper env #t)
				    (pair? (cddr stepper)))
			   (let ((data (var-member (car stepper) vars)))
			     (let ((old-ref (var-ref data)))
			       (lint-walk caller (caddr stepper) inner-env)
			       (set! (var-ref data) old-ref))
			     (if (eq? (car stepper) (caddr stepper))  ; (i 0 i) -> (i 0)
				 (lint-format "perhaps ~A" caller (lists->string stepper (list (car stepper) (cadr stepper)))))
			     ;; pointless caddr here happens very rarely
			     (set! (var-set data) (+ (var-set data) 1))) ; (pair? cddr) above
			   (when (and (pair? (caddr stepper))
				      (not (eq? (car stepper) (cadr stepper))) ; (lst lst (cdr lst))
				      (eq? (car (caddr stepper)) 'cdr)
				      (eq? (cadr stepper) (cadr (caddr stepper))))
			     ;; (do ((x lst (cdr lst))) ((null? x) y))
			     (lint-format "this looks suspicious: ~A" caller stepper))
			   (for-each (lambda (v)
				       (if (and (var-step v)
						(not (eq? (var-name v) (car stepper)))
						(or (eq? (var-name v) (caddr stepper))
						    (and (pair? (caddr stepper))
							 (tree-unquoted-member (var-name v) (caddr stepper)))))
					   (set! baddies (cons (car stepper) baddies))))
				     vars))))
		     
		     (check-unordered-exprs caller form (map var-initial-value vars) env)
		     
		     (when (pair? baddies)
		       ;; (do ((i 0 j) (j ...))...) is unreadable -- which (binding of) j is i set to?
		       ;;    but this is tricky if there is more than one such variable -- if cross links, we'll need named let
		       ;;    and if no step expr, there's no confusion.
		       ;;    (do ((i 0 j) (j 1 i) (k 0 (+ k 1))) ((= k 4)) (format *stderr* "~A ~A~%" i j))
		       ;;    (let __1__ ((i 0) (j 1) (k 0)) (if (= k 4) () (begin (format *stderr* "~A ~A~%" i j) (__1__ j i (+ k 1)))))
		       (let ((new-steppers (map (lambda (stepper)
						  (if (memq (car stepper) baddies)
						      `(,(car stepper) ,(cadr stepper))
						      stepper))
						step-vars))
			     (new-sets (map (lambda (stepper)
					      (if (memq (car stepper) baddies)
						  `(set! ,(car stepper) ,(caddr stepper))
						  (values)))
					    step-vars)))
			 (if (or (null? (cdr baddies))
				 (let ((trails new-sets))
				   (not (any? (lambda (v)     ; for each baddy, is it used in any following set!?
						(and (pair? (cdr trails))
						     (set! trails (cdr trails))
						     (tree-unquoted-member v trails)))
					      (reverse baddies)))))
			     (lint-format "perhaps ~A" caller
					  (lists->string form
							 `(do ,new-steppers
							      ,(caddr form)
							    ,@(cdddr form)
							    ,@new-sets)))
			     ;; (do ((i 0 (+ i j)) (j 0 (+ k 1)) (k 1)) ((= i 10)) (display (+ i j k))) ->
			     ;;    (do ((i 0) (j 0 (+ k 1)) (k 1)) ((= i 10)) (display (+ i j k)) (set! i (+ i j)))
			     (let* ((loop (find-unique-name form))
				    (new-body (let ((let-loop `(,loop ,@(map (lambda (s)
									       ((if (pair? (cddr s)) caddr car) s))
									     step-vars))))
						(if (pair? (cdddr form))
						    `(begin ,@(cdddr form) ,let-loop)
						    let-loop))))
			       (let ((test (if (pair? (caddr form))
					       (caaddr form)
					       ()))
				     (result (if (not (and (pair? (caddr form))
							   (pair? (cdaddr form))))
						 ()
						 (if (null? (cdr (cdaddr form)))
						     (car (cdaddr form))
						     `(begin ,@(cdaddr form))))))
				 ;; (do ((i 0 j) (j 1 i) (k 0 (+ k 1))) ((= k 5) (set! x k) (+ k 1)) (display (+ i j)) -> use named let
				 (lint-format "this do loop is unreadable; perhaps ~A" caller
					      (lists->string form
							     `(let ,loop ,(map (lambda (s)
										 (list (car s) (cadr s)))
									       step-vars)
								   (if ,test ,result ,new-body))))))))))
		   
		   ;; walk the body and end stuff (it's too tricky to find infinite do loops)
		   (when (pair? (caddr form))
		     (let ((end+result (caddr form)))
		       (when (pair? end+result)
			 (let ((end (car end+result)))
			   (lint-walk caller end inner-env) ; this will call simplify-boolean
			   (if (pair? (cdr end+result))
			       (if (null? (cddr end+result))
				   (begin
				     (if (any-null? (cadr end+result))
					 ;; (do ((i 0 (+ i 1))) ((= i 3) ()) (display i))
					 (lint-format "nil return value is redundant: ~A" caller end+result))
				     (lint-walk caller (cadr end+result) inner-env))
				   (lint-walk-open-body caller 'do-result (cdr end+result) inner-env)))
			   (if (and (symbol? end) (memq end '(= > < >= <= null? not)))
			       ;; (do ((i 0 (+ i 1))) (= i 10) (display i))
			       (lint-format "perhaps missing parens: ~A" caller end+result))
			   
			   (cond ((never-false end)
				  ;; (do ((i 0 (+ i 1))) ((+ i 10) i))
				  (lint-format "end test is never false: ~A" caller end))
				 
				 (end ; it's not #f
				  (if (never-true end)
				      (lint-format "end test is never true: ~A" caller end)
				      (let ((v (and (pair? end)
						    (memq (car end) '(< > <= >=))
						    (pair? (cdr end))
						    (symbol? (cadr end))
						    (var-member (cadr end) vars))))
					;; if found, v is the var info
					(when (pair? v)
					  (let ((step (var-step v)))
					    (when (pair? step)
					      (let ((inc (and (memq (car step) '(+ -))
							      (pair? (cdr step))
							      (pair? (cddr step))
							      (or (and (real? (cadr step)) (cadr step))
								  (and (real? (caddr step)) (caddr step))))))
						(when (and (real? inc)
							   (case (car step)
							     ((+) (and (positive? inc) 
								       (memq (car end) '(< <=))))
							     ((-) (and (positive? inc) 
								       (memq (car end) '(> >=))))
							     (else #f)))
						  ;; (do ((i 0 (+ i 1))) ((< i len)) (display i)
						  ;; (do ((i 0 (- i 1))) ((> i len)) (display i))
						  (lint-format "do step looks like it doesn't match end test: ~A" caller 
							       (lists->string step end))))))))))
				 ((pair? (cdr end+result))
				  ;; (do ((i 0 (+ i 1))) (#f i))
				  (lint-format "result is unreachable: ~A" caller end+result)))
			   
			   (if (and (symbol? end)
				    (not (var-member end env))
				    (procedure? (symbol->value end *e*)))
			       ;; (do ((i 0 (+ i 1))) (abs i) (display i))
			       (lint-format "strange do end-test: ~A in ~A is a procedure" caller end end+result))))))
		   
		   (lint-walk-body caller 'do (cdddr form) (cons (make-var :name :let
									   :initial-value form
									   :definer 'do)
								 inner-env))
		   
		   ;; before report-usage, check for unused variables, and don't complain about them if
		   ;;   they are referenced in an earlier step expr.
		   (do ((v vars (cdr v)))
		       ((null? v))
		     (let ((var (car v)))
		       (when (zero? (var-ref var))
			 ;; var was not seen in the end+result/body or any subsequent step exprs
			 ;;   vars is reversed order, so we need only scan var-step of the rest
			 (if (side-effect? (var-step var) env)
			     (set! (var-ref var) (+ (var-ref var) 1))
			     (for-each
			      (lambda (nv)
				(if (or (eq? (var-name var) (var-step nv))
					(and (pair? (var-step nv))
					     (tree-unquoted-member (var-name var) (var-step nv))))
				    (set! (var-ref var) (+ (var-ref var) 1))))
			      (cdr v))))))
		   (report-usage caller 'do vars inner-env)
		   
		   ;; look for constant expressions in the do body
		   (when *report-constant-expressions-in-do*
		     (let ((constant-exprs (find-constant-exprs 'do (map var-name vars) (cdddr form))))
		       (if (pair? constant-exprs)
			   (if (null? (cdr constant-exprs))
			       ;; (do ((p (list 1) (cdr p))) ((null? p)) (set! y (log z 2)) (display x))
			       (lint-format "in ~A, ~A appears to be constant" caller
					    (truncated-list->string form)
					    (car constant-exprs))
			       (lint-format "in ~A, the following expressions appear to be constant:~%~NC~A" caller
					    (truncated-list->string form)
					    (+ lint-left-margin 4) #\space
					    (format #f "~{~A~^, ~}" constant-exprs))))))
		   
		   ;; if simple lambda expanded and exists only for the loop, remove let as well?
		   ;; this can sometimes be simplified
		   (let ((body (cdddr form)))
		     (when (and (pair? body)
				(null? (cdr body))
				(pair? (car body)))
		       (let ((v (var-member (caar body) env)))
			 (when (and (var? v)
				    (memq (var-ftype v) '(define lambda)))
			   (let* ((vfunc (var-initial-value v))
				  (vbody (cddr vfunc)))
			     ;; we already detect a do body with no side-effects (walk-body)
			     (if (and (proper-list? ((if (eq? (var-ftype v) 'define) cdadr cadr) vfunc))
				      (null? (cdr vbody))
				      (< (tree-leaves vbody) 16))
				 (do ((pp (var-arglist v) (cdr pp)))
				     ((or (null? pp)
					  (> (tree-count1 (car pp) vbody 0) 1))
				      (when (null? pp)
					(let ((new-body (copy vbody)))
					  (for-each (lambda (par arg)
						      (if (not (eq? par arg))
							  (set! new-body (tree-subst arg par new-body))))
						    (var-arglist v)
						    (cdar body))
					  ;; (do ((i 0 (+ i 1))) ((= i 10)) (f i)) -> (do ((i 0 (+ i 1))) ((= i 10)) (abs (* 2 i)))
					  (lint-format "perhaps ~A" caller
						       (lists->string form
								      `(do ,(cadr form)
									   ,(caddr form)
									 ,@new-body)))))))))))))
		   
		   ;; do -> for-each
		   ;; TODO: handle two lists
		   (when (and (pair? (cadr form))
			      (null? (cdadr form)))
		     (let ((var (caadr form)))
		       (when (and (pair? (cdr var))
				  (pair? (cddr var))
				  (pair? (caddr var))
				  (eq? (caaddr var) 'cdr)
				  (eq? (car var) (cadr (caddr var)))
				  (pair? (caddr form))
				  (pair? (caaddr form))
				  (null? (cdaddr form)))
			 (let ((vname (car var))
			       (end (caaddr form)))
			   (when (and (case (car end)
					((null?) 
					 (eq? (cadr end) vname))
					((not)
					 (and (pair? (cadr end))
					      (eq? (caadr end) 'pair?)
					      (eq? (cadadr end) vname)))
					(else #f))
				      (not (let walker ((tree (cddr form))) ; since only (cxar sym) is accepted, surely sym can't be shadowed?
					     (and (pair? tree)
						  (or (and (match-cxr 'cdr (car tree))
							   (pair? (cdr tree))
							   (eq? vname (cadr tree)))
						      (and (not (hash-table-ref cxars (car tree)))
							   (pair? (cdr tree))
							   (any? (lambda (p)
								   (or (eq? p vname)
								       (and (pair? p)
									    (walker p))))
								 (cdr tree))))))))
			     ;; this assumes slightly more than the do-loop if (not (pair? var)) is the end-test
			     ;;   for-each wants a sequence, but the do loop checks that in advance.
			     (lint-format "perhaps ~A" caller
					  (lists->string form
							 (let ((new-sym (symbol "[" (symbol->string vname) "]")))
							   `(for-each (lambda (,new-sym)
									,@(car-subst vname new-sym (cdddr form)))
								      ,(cadr var))))))))))
		   
		   ;; check for do-loop as copy/fill! stand-in and other similar cases
		   (when (and (pair? vars)
			      (null? (cdr vars)))
		     (let ((end-test (and (pair? (caddr form)) (caaddr form)))
			   (first-var (caadr form))
			   (body (cdddr form))
			   (setv #f))
		       (when (and (pair? end-test)
				  (pair? body)
				  (null? (cdr body))
				  (pair? (car body)) 
				  (memq (car end-test) '(>= =)))
			 (let ((vname (car first-var))
			       (start (cadr first-var))
			       (step (and (pair? (cddr first-var))
					  (caddr first-var)))
			       (end (caddr end-test)))
			   (when (and (pair? step)
				      (eq? (car step) '+)
				      (memq vname step)
				      (memv 1 step)
				      (null? (cdddr step))
				      (or (eq? (cadr end-test) vname)
					  (and (eq? (car end-test) '=)
					       (eq? (caddr end-test) vname)
					       (set! end (cadr end-test)))))
			     ;; we have (do ((v start (+ v 1)|(+ 1 v))) ((= v end)|(= end v)|(>= v end)) one-statement)
			     (set! body (car body))
			     ;; write-char is the only other common case here -> write-string in a few cases
			     (when (and (memq (car body) '(vector-set! float-vector-set! int-vector-set! list-set! string-set! byte-vector-set!))
					;; integer type check here isn't needed because we're using this as an index below
					;;   the type error will be seen in report-usage if not earlier
					(eq? (caddr body) vname)
					(let ((val (cadddr body)))
					  (set! setv val)
					  (or (code-constant? val)
					      (and (pair? val)
						   (memq (car val) '(vector-ref float-vector-ref int-vector-ref list-ref string-ref byte-vector-ref))
						   (eq? (caddr val) vname)))))
			       ;; (do ((i 2 (+ i 1))) ((= i len)) (string-set! s i #\a)) -> (fill! s #\a 2 len)
			       (lint-format "perhaps ~A" caller 
					    (lists->string form 
							   (if (code-constant? setv)
							       `(fill! ,(cadr body) ,(cadddr body) ,start ,end)
							       `(copy ,(cadr setv) ,(cadr body) ,start ,end))))))))))))
	     env))
	 (hash-table-set! h 'do do-walker))
	
	
	;; ---------------- let ----------------
	(let ()
	 (define (let-walker caller form env)
	   (if (or (< (length form) 3)
		   (not (or (symbol? (cadr form))
			    (list? (cadr form)))))
	       ;; (let ((a 1) (set! a 2)))
	       (lint-format "let is messed up: ~A" caller (truncated-list->string form))
	       (let ((named-let (and (symbol? (cadr form)) (cadr form))))
		 (if (keyword? named-let)
		     ;; (let :x ((i y)) (x i))
		     (lint-format "bad let name: ~A" caller named-let))

		 (unless named-let
		   
		   (if (and (null? (cadr form)) ; this can be fooled by macros that define things
			    (eq? form lint-current-form) ; i.e. we're in a body?
			    (not (tree-set-member '(call/cc call-with-current-continuation lambda lambda* define define* 
							    define-macro define-macro* define-bacro define-bacro* define-constant define-expansion
							    load eval eval-string require)
						  (cddr form))))
		       ;; (begin (let () (display x)) y)
		       (lint-format "pointless let: ~A" caller (truncated-list->string form))
		       (let ((body (cddr form)))
			 (when (and (null? (cdr body))
				    (pair? (car body)))
			   (if (memq (caar body) '(let let*))
			       (if (null? (cadr form))
				   ;; (let () (let ((a x)) (+ a 1)))
				   (lint-format "pointless let: ~A" caller (lists->string form (car body)))
				   (if (null? (cadar body))
				       ;; (let ((a x)) (let () (+ a 1)))
				       (lint-format "pointless let: ~A" caller (lists->string form `(let ,(cadr form) ,@(cddar body))))))
			       (if (and (memq (caar body) '(lambda lambda*)) ; or any definer?
					(null? (cadr form)))
				   ;; (let () (lambda (a b) (if (positive? a) (+ a b) b))) -> (lambda (a b) (if (positive? a) (+ a b) b))
				   (lint-format "pointless let: ~A" caller (lists->string form (car body)))))))))
		 
		 (let ((vars (if (or (not named-let)
				     (keyword? named-let)
				     (not (or (null? (caddr form))
					      (and (proper-list? (caddr form))
						   (every? pair? (caddr form))))))
				 ()
				 (list (make-fvar :name named-let 
						  :ftype 'let
						  :decl (dummy-func caller form (list 'define (cons '_ (map car (caddr form))) #f))
						  :arglist (map car (caddr form))
						  :initial-value form
						  :env env))))
		       (varlist ((if named-let caddr cadr) form))
		       (body ((if named-let cdddr cddr) form)))
		   
		   (if (not (list? varlist))
		       (lint-format "let is messed up: ~A" caller (truncated-list->string form))
		       (if (and (null? varlist)
				(pair? body)
				(null? (cdr body))
				(not (side-effect? (car body) env)))
			   ;; (let xx () z)
			   (lint-format "perhaps ~A" caller (lists->string form (car body)))))
		   
		   (do ((bindings varlist (cdr bindings)))
		       ((not (pair? bindings))
			(if (not (null? bindings))
			    ;; (let ((a 1) . b) a)
			    (lint-format "let variable list is not a proper list? ~S" caller varlist)))
		     (when (binding-ok? caller 'let (car bindings) env #f)
		       (let ((val (cadar bindings)))
			 (if (and (pair? val)
				  (eq? 'lambda (car val))
				  (tree-car-member (caar bindings) val)
				  (not (var-member (caar bindings) env)))
			     ;; (let ((x (lambda (a) (x 1)))) x)
			     (lint-format "let variable ~A is called in its binding?  Perhaps let should be letrec: ~A"
					  caller (caar bindings) 
					  (truncated-list->string bindings))
			     (unless named-let
			       (for-each (lambda (v)
					   (if (and (tree-memq (var-name v) (cadar bindings))
						    (not (hash-table-ref built-in-functions (var-name v)))
						    (not (tree-table-member binders (cadar bindings))))
					       (if (not (var-member (var-name v) env))
						   ;; (let ((x 1) (y x)) (+ x y)): x in (y x)
						   (lint-format "~A in ~A does not appear to be defined in the calling environment" caller
								(var-name v) (car bindings))
						   ;; (let ((x 3)) (+ x (let ((x 1) (y x)) (+ x y)))): x in (y x)
						   (lint-format "~A in ~A refers to the caller's ~A, not the let variable" caller
								(var-name v) (car bindings) (var-name v)))))
					 vars)))
			 (lint-walk caller val env)
			 (set! vars (cons (make-var :name (caar bindings) 
						    :initial-value val 
						    :definer (if named-let 'named-let 'let))
					  vars)))))
		   
		   (check-unordered-exprs caller form 
					  (map (if (not named-let)
						   var-initial-value
						   (lambda (v)
						     (if (eq? (var-name v) named-let)
							 (values)
							 (var-initial-value v))))
					       vars)
					  env)
		   
		   (let ((suggest made-suggestion))
		     (when (and (pair? varlist)          ; (let ((x (A))) (if x (f x) B)) -> (cond ((A) => f) (else B)
				(pair? body)
				(pair? (car body))
				(null? (cdr body))
				(pair? (cdar body)))
		       
		       (when (and (pair? (car varlist))  ;   ^ this happens a lot, so it's worth this tedious search
				  (null? (cdr varlist))  ;   also (let ((x (A))) (cond (x (f x))...)
				  (pair? (cdar varlist))
				  (pair? (cadar varlist)))
			 
			 (let ((p (car body))
			       (vname (caar varlist))
			       (vvalue (cadar varlist)))
			   
			   (when (and (not named-let)    ; (let ((x (assq a y))) (set! z (if x (cadr x) 0))) -> (set! z (cond ((assq a y) => cadr) (else 0)))
				      (not (memq (car p) '(if cond))) ; handled separately below
				      (= (tree-count2 vname p 0) 2))
			     (do ((i 0 (+ i 1))
				  (bp (cdr p) (cdr bp)))
				 ((or (null? bp)
				      (let ((b (car bp)))
					(and (pair? b)
					     (eq? (car b) 'if)
					     (= (tree-count2 vname b 0) 2)
					     (eq? vname (cadr b))
					     (pair? (caddr b))
					     (pair? (cdaddr b))
					     (null? (cddr (caddr b)))
					     (eq? vname (cadr (caddr b))))))
				  (if (pair? bp)
				      (let ((else-clause (if (pair? (cdddar bp)) `((else ,@(cdddar bp))) ())))
					(lint-format "perhaps ~A" caller
						     (lists->string form `(,@(copy p (make-list (+ i 1)))
									   (cond (,vvalue => ,(caaddr (car bp))) ,@else-clause)
									   ,@(cdr bp)))))))))
			   
			   (when (and (eq? (car p) 'cond) ; (let ((x (f y))) (cond (x (g x)) ...)) -> (cond ((f y) => g) ...)
				      (pair? (cadr p))
				      (eq? (caadr p) vname)
				      (pair? (cdadr p))
				      (null? (cddadr p))
				      (or (and (pair? (cadadr p))
					       (pair? (cdr (cadadr p)))
					       (null? (cddr (cadadr p))) ; one arg to func
					       (eq? vname (cadr (cadadr p))))
					  (eq? vname (cadadr p)))
				      (or (null? (cddr p))
					  (not (tree-unquoted-member vname (cddr p)))))
			     (lint-format "perhaps ~A" caller 
					  (lists->string form 
							 (if (eq? vname (cadadr p))
							     (if (and (pair? (cddr p))
								      (pair? (caddr p))
								      (memq (caaddr p) '(else #t t)))
								 (if (null? (cddr (caddr p)))
								     `(or ,vvalue ,(cadr (caddr p)))
								     `(or ,vvalue (begin ,@(cdaddr p))))
								 `(or ,vvalue 
								      (cond ,@(cddr p))))
							     `(cond (,vvalue => ,(caadr (cadr p))) 
								    ,@(cddr p))))))
			   
			   (when (and (null? (cddr p))         ; (let ((x (+ y 1))) (abs x)) -> (abs (+ y 1))
				      (eq? vname (cadr p)))    ;   not tree-subst or trailing (pair) args: the let might be forcing evaluation order
			     (let ((v (var-member (car p) env)))
			       (if (or (and (var? v)
					    (memq (var-definer v) '(define define* lambda lambda*)))
				       (hash-table-ref built-in-functions (car p)))
				   (lint-format "perhaps ~A" caller (lists->string form `(,(car p) ,vvalue)))
				   (if (not (or (any-macro? vname env)
						(tree-unquoted-member vname (car p))))
				       (lint-format "perhaps, assuming ~A is not a macro, ~A" caller (car p)
						    (lists->string form `(,(car p) ,vvalue)))))))
			   
			   (when (pair? (cddr p))
			     (when (and (eq? (car p) 'if)
					(pair? (cdddr p)))
			       (let ((if-true (caddr p))
				     (if-false (cadddr p)))
				 
				 (when (and (eq? (cadr p) vname) ; (let ((x (g y))) (if x #t #f)) -> (g y)
					    (boolean? if-true)
					    (boolean? if-false)
					    (not (eq? if-true if-false)))
				   (lint-format "perhaps ~A" caller
						(lists->string form (if if-true vvalue `(not ,vvalue)))))
				 
				 (when (and (pair? (cadr p)) ; (let ((x (f y))) (if (not x) B (g x))) -> (cond ((f y) => g) (else B))
					    (eq? (caadr p) 'not)
					    (eq? (cadadr p) vname)
					    (pair? if-false)
					    (pair? (cdr if-false))
					    (null? (cddr if-false))
					    (eq? vname (cadr if-false)))
				   (let ((else-clause (if (eq? if-true vname)
							  `((else #f))
							  (if (and (pair? if-true)
								   (tree-unquoted-member vname if-true))
							      :oops! ; if the let var appears in the else portion, we can't do anything with =>
							      `((else ,if-true))))))
				     (unless (eq? else-clause :oops!)
				       (lint-format "perhaps ~A" caller (lists->string form `(cond (,vvalue => ,(car if-false)) ,@else-clause))))))))
			     
			     (let ((crf #f))
			       ;; all this stuff still misses (cond ((not x)...)) and (set! y (if x (cdr x)...)) i.e. need embedding in this case
			       (when (and (or (and (memq (car p) '(if and)) ; (let ((x (f y))) (and x (g x))) -> (cond ((f y) => g) (else #f))
						   (eq? (cadr p) vname))
					      (and (eq? (car p) 'or)
						   (equal? (cadr p) `(not ,vname)))
					      (and (pair? vvalue)
						   (memq (car vvalue) '(assoc assv assq member memv memq))
						   (pair? (cadr p))
						   (or (eq? (caadr p) 'pair?)
						       (and (eq? (caadr p) 'null?)
							    ;; (let ((x (assoc y z))) (if (null? x) (g x)))
							    (lint-format "in ~A, ~A can't be null because ~A in ~A only returns #f or a pair" 
									 caller p vname (car vvalue) (truncated-list->string (car varlist)))
							    #f))
						   (eq? (cadadr p) vname)))
					  
					  (or (and (pair? (caddr p))
						   (pair? (cdaddr p))
						   (null? (cddr (caddr p))) ; one func arg
						   (or (eq? vname (cadr (caddr p)))
						       (and (hash-table-ref combinable-cxrs (caaddr p))
							    ((lambda* (cr arg) ; lambda* not lambda because combine-cxrs might return just #f
							       (and cr
								    (< (length cr) 5) 
								    (eq? vname arg)
								    (set! crf (symbol "c" cr "r"))))
							     (combine-cxrs (caddr p))))))
					      (and (eq? (car p) 'if)
						   (eq? (caddr p) vname)
						   (not (tree-unquoted-member vname (cdddr p)))
						   ;; (let ((x (g y))) (if x x (g z))) -> (or (g y) (g z))
						   (lint-format "perhaps ~A" caller 
								(lists->string form 
									       (if (null? (cdddr p))
										   vvalue
										   `(or ,vvalue ,(cadddr p)))))
						   #f))
					  (pair? (caddr p))
					  (or (eq? (car p) 'if)
					      (null? (cdddr p))))
				 (let ((else-clause (if (pair? (cdddr p))
							(if (eq? (cadddr p) vname)
							    `((else #f)) ; this stands in for the local var
							    (if (and (pair? (cadddr p))
								     (tree-unquoted-member vname (cadddr p)))
								:oops! ; if the let var appears in the else portion, we can't do anything with =>
								`((else ,(cadddr p)))))
							(case (car p)
							  ((and) '((else #f)))
							  ((or)  '((else #t)))
							  (else  ())))))
				   (unless (eq? else-clause :oops!)
				     ;; (let ((x (assoc y z))) (if x (cdr x))) -> (cond ((assoc y z) => cdr))
				     (lint-format "perhaps ~A" caller 
						  (lists->string form `(cond (,vvalue => ,(or crf (caaddr p))) ,@else-clause))))))))
			   )) ; one var in varlist
		       
		       ;; (let ((x 1) (y 2)) (+ x y)) -> (+ 1 2)
		       ;; this happens a lot, but it often looks like a form of documentation
		       (when (and (= suggest made-suggestion)
				  (not named-let)
				  (< (length varlist) 8)
				  (not (memq (caar body) '(lambda lambda* define define* define-macro)))
				  (not (and (eq? (caar body) 'set!)
					    (any? (lambda (v) (eq? (car v) (cadar body))) varlist)))
				  (not (any-macro? (caar body) env))
				  (not (any? (lambda (p)
					       (and (pair? p)
						    (not (eq? (car p) 'quote))
						    (or (not (hash-table-ref no-side-effect-functions (car p)))
							(any? pair? (cdr p)))))
					     (cdar body)))
				  (every? (lambda (v)
					    (and (pair? v)
						 (pair? (cdr v))
						 (< (tree-leaves (cadr v)) 8)
						 (= (tree-count1 (car v) body 0) 1)))
					  varlist))
			 (let ((new-body (copy (car body)))
			       (bool-arg? #f))
			   (for-each (lambda (v)
				       (if (not bool-arg?)
					   (let tree-walk ((tree body))
					     (if (pair? tree)
						 (if (and (memq (car tree) '(or and))
							  (memq (car v) (cdr tree)))
						     (set! bool-arg? #t)
						     (begin
						       (tree-walk (car tree))
						       (tree-walk (cdr tree)))))))
				       (set! new-body (tree-subst (cadr v) (car v) new-body)))
				     varlist)
			   (lint-format (if bool-arg? 
					    "perhaps, ignoring short-circuit issues, ~A"
					    "perhaps ~A")
					caller (lists->string form new-body))))
		       ) ; null cdr body etc
		     
		     (when (and (pair? (cadr form))       ; (let ((x x)) (+ x 1)) -> (+ x 1), (let ((x x))...) does not copy x if x is a sequence
				(= suggest made-suggestion)
				(every? (lambda (c)       
					  (and (pair? c)  ; the usual... (let binding might be messed up)
					       (pair? (cdr c)) 
					       (eq? (car c) (cadr c))))
					(cadr form))
				(not (and (pair? (caddr form))
					  (memq (caaddr form) '(lambda lambda*)))))
		       (let ((vs (map car (cadr form))))
			 (unless (any? (lambda (p) 
					 (and (pair? p)
					      (memq (cadr p) vs)
					      (or (eq? (car p) 'set!)
						  (set!? p env))))
				       (cddr form))
			   (lint-format "perhaps omit this useless let: ~A" caller
					(truncated-lists->string form
								 (if (null? (cdddr form))
								     (caddr form)
								     `(begin ,@(cddr form))))))))
		     ) ; suggest let
		   
		   (let* ((cur-env (cons (make-var :name :let
						   :initial-value form
						   :definer 'let)
					 (append vars env)))
			  (e (lint-walk-body (or named-let caller) 'let body cur-env)))

		     (let ((nvars (and (not (eq? e cur-env))
				       (env-difference caller e cur-env ()))))
		       (if (pair? nvars)
			   (if (memq (var-name (car nvars)) '(:lambda :dilambda))
			       (begin
				 (set! env (cons (car nvars) env))
				 (set! nvars (cdr nvars)))
			       (set! vars (append nvars vars)))))
		     
		     (if (and (pair? body)
			      (equal? (list-ref body (- (length body) 1)) '(curlet))) ; the standard library tag
			 (for-each (lambda (v)
				     (set! (var-ref v) (+ (var-ref v) 1)))
				   e))
		     
		     (report-usage caller 'let vars cur-env)

		     ;; look for splittable lets and let-temporarily possibilities
		     (when (and (pair? vars)
				(pair? (cadr form))
				(pair? (caadr form)))
		       (for-each 
			(lambda (local-var)
			  (let ((vname (var-name local-var)))
			    
			    ;; ideally we'd collect vars that fit into one let etc
			    (when (> (length body) (* 5 (var-set local-var)) 0)
			      (do ((i 0 (+ i 1))
				   (preref #f)
				   (p body (cdr p)))
				  ((or (not (pair? (cdr p)))
				       (and (pair? (car p))
					    (eq? (caar p) 'set!)
					    (eq? (cadar p) vname)
					    (> i 5)
					    (begin
					      (if (or preref
						      (side-effect? (var-initial-value local-var) env))
						  ;; (let ((x 32)) (display x) (set! y (f x)) (g (+ x 1) y) (a y) (f y) (g y) (h y) (i y) (set! x 3) (display x) (h y x))
						  ;;     (let ... (let ((x 3)) ...))
						  (lint-format "perhaps add a new binding for ~A to replace ~A: ~A" caller
							       vname
							       (truncated-list->string (car p))
							       (lists->string form
									      `(let ...
										 (let ((,vname ,(caddar p)))
										   ...))))
						  ;; (let ((x 32)) (set! y (f 1)) (a y) (f y) (g y) (h y) (i y) (set! x (+ x... -> (let () ... (let ((x (+ 32 1))) ...))
						  (lint-format "perhaps move the ~A binding to replace ~A: ~A" caller
							       vname 
							       (truncated-list->string (car p))
							       (let ((new-value (if (tree-member vname (caddar p))
										    (tree-subst (var-initial-value local-var) vname (copy (caddar p)))
										    (caddar p))))
								 (lists->string form 
										`(let ,(let rewrite ((lst (cadr form)))
											 (cond ((null? lst) ())
											       ((and (pair? (car lst))
												     (eq? (caar lst) vname))
												(rewrite (cdr lst)))
											       (else (cons (if (< (tree-leaves (cadar lst)) 30)
													       (car lst)
													       (list (caar lst) '...))
													   (rewrite (cdr lst))))))
										   ...
										   (let ((,vname ,new-value))
										     ...))))))
					      #t))))
				(if (tree-member vname (car p))
				    (set! preref i))))
			    
			    (when (and (zero? (var-set local-var))
				       (= (var-ref local-var) 2) ; initial value and set!
				       (symbol? (var-initial-value local-var)))
			      (do ((saved-name (var-initial-value local-var))
				   (p body (cdr p))
				   (last-pos #f)
				   (first-pos #f))
				  ((not (pair? p))
				   (when (and (pair? last-pos)
					      (not (eq? first-pos last-pos))
					      (not (tree-member saved-name (cdr last-pos))))
				     ;; (let ((old-x x)) (set! x 12) (display (log x)) (set! x 1) (set! x old-x)) ->
				     ;;    (let-temporarily ((x 12)) (display (log x)) (set! x 1))
				     (lint-format "perhaps use let-temporarily here (see stuff.scm): ~A" caller
						  (lists->string form
								 (let ((new-let `(let-temporarily 
										     ((,saved-name ,(if (pair? first-pos) 
													(caddar first-pos) 
													saved-name)))
										   ,@(map (lambda (expr)
											    (if (or (and (pair? first-pos)
													 (eq? expr (car first-pos)))
												    (eq? expr (car last-pos)))
												(values)
												expr))
											  body))))
								   (if (null? (cdr vars)) ; we know vars is a pair, want len=1
								       new-let
								       `(let ,(map (lambda (v)
										     (if (eq? (car v) vname)
											 (values)
											 v))
										   (cadr form))
									  ,new-let)))))))
				;; someday maybe look for additional saved vars, but this happens only in snd-test
				;;   also the let-temp could be reduced to the set locations (so the tree-member
				;;   check above would be unneeded).
				(let ((expr (car p)))
				  (when (and (pair? expr)
					     (eq? (car expr) 'set!)
					     (eq? (cadr expr) saved-name)
					     (pair? (cddr expr)))
				    (if (not first-pos)
					(set! first-pos p))
				    (if (eq? (caddr expr) vname)
					(set! last-pos p))))))))
			vars)))
		   
		   (when (and (pair? varlist)
			      (pair? (car varlist))
			      (null? (cdr varlist)))

		     (if (and (pair? body)                ; (let ((x y)) x) -> y, named let is possible here 
			      (null? (cdr body))
			      (eq? (car body) (caar varlist))
			      (pair? (cdar varlist)))     ; (let ((a))...)
			 (lint-format "perhaps ~A" caller (lists->string form (cadar varlist))))
		     ;; also (let ((x ...)) (let ((y x)...))) happens but it looks like automatically generated code or test suite junk
		     
		     ;; copied from letrec below -- happens about a dozen times
		     (when (and (not named-let)
				(pair? (cddr form))
				(pair? (caddr form))
				(null? (cdddr form)))
		       (let ((body (caddr form))
			     (sym (caar varlist))
			     (lform (and (pair? (caadr form))
					 (pair? (cdaadr form))
					 (cadar (cadr form)))))
			 (if (and (pair? lform)
				  (pair? (cdr lform))
				  (eq? (car lform) 'lambda)
				  (proper-list? (cadr lform)))
			     ;; unlike in letrec, here there can't be recursion (ref to same name is ref to outer env)
			     (if (eq? sym (car body))
				 (if (not (tree-memq sym (cdr body)))
				     ;; (let ((x (lambda (y) (+ y (x (- y 1)))))) (x 2)) -> (let ((y 2)) (+ y (x (- y 1))))
				     (lint-format "perhaps ~A" caller
						  (lists->string 
						   form `(let ,(map list (cadr lform) (cdr body))
							   ,@(cddr lform)))))
				 (if (= (tree-count1 sym body 0) 1)
				     (let ((call (find-call sym body)))
				       (when (pair? call) 
					 (let ((new-call `(let ,(map list (cadr lform) (cdr call))
							    ,@(cddr lform))))
					   ;; (let ((f60 (lambda (x) (* 2 x)))) (+ 1 (f60 y))) -> (+ 1 (let ((x y)) (* 2 x)))
					   (lint-format "perhaps ~A" caller
							(lists->string form (tree-subst new-call call body))))))))))))
		   (when (pair? body)
		     (when (and (pair? (car body))
				(pair? (cdar body))
				(pair? (cddar body))      
				(eq? (caar body) 'set!))
		       (let ((settee (cadar body))
			     (setval (caddar body)))
			 (if (and (not named-let)           ; (let ((x 0)...) (set! x 1)...) -> (let ((x 1)...)...)
				  (not (tree-memq 'curlet setval))
				  (cond ((assq settee vars)
					 => (lambda (v)
					      (or (and (code-constant? (var-initial-value v))
						       (code-constant? setval))
						  (not (any? (lambda (v1)
							       (or (tree-memq (car v1) setval)
								   (side-effect? (cadr v1) env)))
							     varlist)))))
					(else #f)))
			     (lint-format "perhaps ~A" caller  ;  (let ((a 1)) (set! a 2)) -> 2
					  (lists->string form 
							 (if (null? (cdr body)) ; this only happens in test suites...
							     (if (null? (cdr varlist))
								 setval
								 `(let ,(map (lambda (v) (if (eq? (car v) settee) (values) v)) varlist)
								    ,setval))
							     `(let ,(map (lambda (v)
									   (if (eq? (car v) settee)
									       (list (car v) setval)
									       v))
									 varlist)
								,@(if (null? (cddr body))
								      (cdr body)
								      `(,(cadr body) ...))))))
			     ;; repetition for the moment
			     (when (and (pair? varlist)
					(assq settee vars)           ; settee is a local var
					(not (eq? settee named-let)) ; (let loop () (set! loop 3))!
					(or (null? (cdr body))
					    (and (null? (cddr body))
						 (eq? settee (cadr body))))) ; (let... (set! local val) local)
			       (lint-format "perhaps ~A" caller
					    (lists->string form
							   (if (or (tree-memq settee setval)
								   (side-effect? (cadr (assq settee varlist)) env))
							       `(let ,varlist ,setval)
							       (if (null? (cdr varlist))
								   setval
								   `(let ,(remove-if (lambda (v)
										       (eq? (car v) settee))
										     varlist)
								      ,setval)))))))))
		     (unless named-let
		       
		       ;; if var val is symbol, val not used (not set!) in body (even hidden via function call)
		       ;;   and var not set!, and not a function parameter (we already reported those),
		       ;;   remove it (the var) and replace with val throughout

		       (when (and (proper-list? (cadr form))
				  (not (tree-set-member '(curlet lambda lambda* define define*) (cddr form))))
			 (do ((changes ())
			      (vs (cadr form) (cdr vs)))
			     ((null? vs)
			      (if (pair? changes)
				  (let ((new-form (copy form)))
				    (for-each 
				     (lambda (v)
				       (list-set! new-form 1 (remove-if (lambda (p) (equal? p v)) (cadr new-form)))
				       (set! new-form (tree-subst (cadr v) (car v) new-form)))
				     changes)
				    (lint-format "assuming we see all set!s, the binding~A ~{~A~^, ~} ~A pointless: perhaps ~A" caller
						 (if (pair? (cdr changes)) "s" "")
						 changes 
						 (if (pair? (cdr changes)) "are" "is")
						 (lists->string form 
								(if (< (tree-leaves new-form) 200)
								    new-form
								    `(let ,(cadr new-form)
								       ,@(one-call-and-dots (cddr new-form)))))))))
			   (let ((v (car vs)))
			     (if (and (pair? v)
				      (pair? (cdr v))
				      (null? (cddr v))                  ; good grief
				      (symbol? (cadr v))
				      (not (set-target (cadr v) body env))
				      (not (set-target (car v) body env))
				      (let ((data (var-member (cadr v) env)))
					(or (not (var? data))
					    (and (not (eq? (var-definer data) 'parameter))
						 (or (null? (var-setters data))
						     (not (tree-set-member (var-setters data) body)))))))
				 (set! changes (cons v changes))))))
		       
		       (when (pair? varlist)

			 ;; if last is (set! local-var...) and no complications, complain
			 (let ((last (list-ref body (- (length body) 1))))
			   (if (and (pair? last)
				    (eq? (car last) 'set!)
				    (pair? (cdr last))
				    (pair? (cddr last)) ; (set! a)
				    (symbol? (cadr last))
				    (assq (cadr last) varlist)       ; (let ((a 1) (b (display 2))) (set! a 2))
				    ;; this is overly restrictive:
				    (not (tree-set-member '(call/cc call-with-current-continuation curlet lambda lambda*) form)))
			       (lint-format "set! is pointless in ~A: use ~A" caller
					    last (caddr last))))

			 (when (and (pair? (car body))
				    (eq? (caar body) 'do))
			   (when (and (null? (cdr body))  ; removing this restriction gets only 3 hits
				      (pair? (cadar body)))
			     (let ((inits (map cadr (cadar body))))
			       (when (every? (lambda (v)
					       (and (= (tree-count1 (car v) (car body) 0) 1)
						    (tree-memq (car v) inits)))
					     varlist)
				 (let ((new-cadr (copy (cadar body))))
				   (for-each (lambda (v)
					       (set! new-cadr (tree-subst (cadr v) (car v) new-cadr)))
					     varlist)
				   ;; (let ((a 1)) (do ((i a (+ i 1))) ((= i 3)) (display i))) -> (do ((i 1 (+ i 1))) ...)
				   (lint-format "perhaps ~A" caller
						(lists->string form `(do ,new-cadr ...)))))))
			   
			   ;; let->do -- sometimes a bad idea, set *max-cdr-len* to #f to disable this.
			   ;;   (the main objection is that the s7/clm optimizer can't handle it, and
			   ;;   instruments using it look kinda dumb -- the power of habit or something)
			   (when (integer? *max-cdr-len*)
			     (let ((inits (if (pair? (cadar body))
					      (map cadr (cadar body))
					      ()))
				   (locals (if (pair? (cadar body))
					       (map car (cadar body))
					       ())))
			       (unless (or (and (pair? inits)
						(any? (lambda (v)
							(or (memq (car v) locals) ; shadowing
							    (tree-memq (car v) inits)
							    (side-effect? (cadr v) env))) ; let var opens *stdin*, do stepper reads it at init
						      varlist))
					   (> (tree-leaves (cdr body)) *max-cdr-len*))
				 ;; (let ((xx 0)) (do ((x 1 (+ x 1)) (y x (- y 1))) ((= x 3) xx) (display y))) ->
				 ;;    (do ((xx 0) (x 1 (+ x 1)) (y x (- y 1))) ...)
				 (lint-format "perhaps ~A" caller
					      (lists->string form
							     (let ((do-form (cdar body)))
							       (if (null? (cdr body))    ; do is only expr in let
								   `(do ,(append varlist (car do-form))
									...)
								   `(do ,(append varlist (car do-form))
									(,(and (pair? (cadr do-form)) (caadr do-form))
									 ,@(if (side-effect? (cdr (cadr do-form)) env) (cdr (cadr do-form)) ())
									 ,@(cdr body))   ; include rest of let as do return value
								      ...)))))))))
			 
			 (when (and (> (length body) 3)  ; setting this to 1 did not catch anything new
				    (every? pair? varlist)
				    (not (tree-set-car-member '(define define* define-macro define-macro* 
								 define-bacro define-bacro* define-constant define-expansion) 
							      body)))
			   ;; define et al are like a continuation of the let bindings, so we can't restrict them by accident
			   ;;   (let ((x 1)) (define y x) ...)
			   (let ((last-refs (map (lambda (v) 
						   (vector (var-name v) #f 0 v))
						 vars))
				 (got-lambdas (tree-set-car-member '(lambda lambda*) body)))
			     ;; (let ((x #f) (y #t)) (set! x (lambda () y)) (set! y 5) (x))
			     (do ((p body (cdr p))
				  (i 0 (+ i 1)))
				 ((null? p)
				  (let ((end 0))
				    (for-each (lambda (v)
						(set! end (max end (v 2))))
					      last-refs)
				    (if (and (< end (/ i lint-let-reduction-factor))
					     (eq? form lint-current-form)
					     (< (tree-leaves (car body)) 100))
					(let ((old-start (let ((old-pp ((funclet lint-pretty-print) '*pretty-print-left-margin*)))
							   (set! ((funclet lint-pretty-print) '*pretty-print-left-margin*) (+ lint-left-margin 4))
							   (let ((res (lint-pp `(let ,(cadr form) 
										  ,@(copy body (make-list (+ end 1)))))))
							     (set! ((funclet lint-pretty-print) '*pretty-print-left-margin*) old-pp)
							     res))))
					  (lint-format "this let could be tightened:~%~NC~A ->~%~NC~A~%~NC~A ..." caller
						       (+ lint-left-margin 4) #\space
						       (truncated-list->string form)
						       (+ lint-left-margin 4) #\space
						       old-start
						       (+ lint-left-margin 4) #\space
						       (lint-pp (list-ref body (+ end 1)))))
					(begin
					  ;; look for bindings that can be severely localized 
					  (let ((locals (map (lambda (v)
							       (if (and (integer? (v 1))
									(< (- (v 2) (v 1)) 2)
									(code-constant? (var-initial-value (v 3))))
								   v
								   (values)))
							     last-refs)))
					    ;; should this omit cases where most the let is in the one or two lines?
					    (when (pair? locals)
					      (set! locals (sort! locals (lambda (a b) 
									   (or (< (a 1) (b 1))
									       (< (a 2) (b 2))))))
					      (do ((lv locals (cdr lv)))
						  ((null? lv))
						(let* ((v (car lv))
						       (cur-line (v 1)))
						  (let gather ((pv lv) (cur-vars ()) (max-line (v 2)))
						    (if (or (null? (cdr pv))
							    (not (= cur-line ((cadr pv) 1))))
							(begin
							  (set! cur-vars (reverse (cons (car pv) cur-vars)))
							  (set! max-line (max max-line ((car pv) 2)))
							  (set! lv pv)
							  (lint-format "~{~A~^, ~} ~A only used in expression~A (of ~A),~%~NC~A~A of~%~NC~A" caller
								       (map (lambda (v) (v 0)) cur-vars)
								       (if (null? (cdr cur-vars)) "is" "are")
								       (if (= cur-line max-line)
									   (format #f " ~D" (+ cur-line 1))
									   (format #f "s ~D and ~D" (+ cur-line 1) (+ max-line 1)))
								       (length body)
								       (+ lint-left-margin 6) #\space
								       (truncated-list->string (list-ref body cur-line))
								       (if (= cur-line max-line)
									   ""
									   (format #f "~%~NC~A" 
										   (+ lint-left-margin 6) #\space
										   (truncated-list->string (list-ref body max-line))))
								       (+ lint-left-margin 4) #\space
								       (truncated-list->string form)))
							(gather (cdr pv) 
								(cons (car pv) cur-vars) 
								(max max-line ((car pv) 2)))))))))
					  (let ((mnv ())
						(cur-end i))
					    (for-each (lambda (v)
							(when (and (or (null? mnv)
								       (<= (v 2) cur-end))
								   (positive? (var-ref (v 3)))
								   (let ((expr (var-initial-value (v 3))))
								     (not (any? (lambda (ov) ; watch out for shadowed vars
										  (tree-memq (car ov) expr))
										varlist))))
							  (set! mnv (if (= (v 2) cur-end)
									(cons v mnv)
									(list v)))
							  (set! cur-end (v 2))))
						      last-refs)
					    
					    ;; look for vars used only at the start of the let
					    (when (and (pair? mnv)
						       (< cur-end (/ i lint-let-reduction-factor))
						       (> (- i cur-end) 3))
					      ;; mnv is in the right order because last-refs is reversed
					      (lint-format "the scope of ~{~A~^, ~} could be reduced: ~A" caller 
							   (map (lambda (v) (v 0)) mnv)
							   (lists->string form
									  `(let ,(map (lambda (v)
											(if (member (car v) mnv (lambda (a b) (eq? a (b 0))))
											    (values)
											    v))
										      varlist)
									     (let ,(map (lambda (v)
											  (list (v 0) (var-initial-value (v 3))))
											mnv)
									       ,@(copy body (make-list (+ cur-end 1))))
									     ,(list-ref body (+ cur-end 1))
									     ...)))))))))
			       
			       ;; body of do loop above
			       (if (and (not got-lambdas)
					(pair? (car p))
					(pair? (cdr p))
					(eq? (caar p) 'set!)
					(var-member (cadar p) vars)
					(not (tree-memq (cadar p) (cdr p))))
				   (if (not (side-effect? (caddar p) env))     ;  (set! v0 (channel->vct 1000 100)) -> (channel->vct 1000 100)
				       (lint-format "~A in ~A could be omitted" caller (car p) (truncated-list->string form))
				       (lint-format "perhaps ~A" caller (lists->string (car p) (caddar p)))))
			       ;; 1 use in cadr and none thereafter happens a few times, but looks like set-as-documentation mostly
			       
			       (for-each (lambda (v)
					   (when (tree-memq (v 0) (car p))
					     (set! (v 2) i)
					     (if (not (v 1)) (set! (v 1) i))))
					 last-refs))))))
		     ) ; (when (pair? body)...)
		   
		   ;; out of place and repetitive code...
		   (when (and (pair? (cadr form))
			      (pair? (cddr form))
			      (null? (cdddr form))
			      (pair? (caddr form)))
		     (let ((inner (caddr form))  ; the inner let
			   (outer-vars (cadr form)))
		       
		       (when (pair? (cdr inner))
			 (let ((inner-vars (cadr inner)))
			   (when (and (eq? (car inner) 'let)
				      (symbol? inner-vars))
			     (let ((named-body (cdddr inner))
				   (named-args (caddr inner)))
			       (unless (any? (lambda (v)
					       (or (not (= (tree-count1 (car v) named-args 0) 1))
						   (tree-memq (car v) named-body)))
					     varlist)
				 (let ((new-args (copy named-args)))
				   (for-each (lambda (v)
					       (set! new-args (tree-subst (cadr v) (car v) new-args)))
					     varlist)
				   ;; (let ((x 1) (y (f g 2))) (let loop ((a (+ x 1)) (b y)) (loop a b))) -> (let loop ((a (+ 1 1)) (b (f g 2))) (loop a b))
				   (lint-format "perhaps ~A" caller
						(lists->string form
							       `(let ,inner-vars ,new-args ,@named-body)))))))
			   
			   ;; maybe more code than this is worth -- combine lets
			   (when (and (memq (car inner) '(let let*))
				      (pair? inner-vars))
			     
			     (define (letstar . lets)
			       (let loop ((vars (list 'curlet)) (forms lets))
				 (and (pair? forms)
				      (or (and (pair? (car forms))
					       (or (tree-set-member vars (car forms))
						   (any? (lambda (a) 
							   (or (not (pair? a))
							       (not (pair? (cdr a))) 
							       (side-effect? (cadr a) env)))
							 (car forms))))
					  (loop (append (map car (car forms)) vars) 
						(cdr forms))))))
			     
			     (cond ((and (null? (cdadr form))   ; let(1) + let* -> let*
					 (eq? (car inner) 'let*)
					 (not (symbol? inner-vars))) ; not named let*
				    ;; (let ((a 1)) (let* ((b (+ a 1)) (c (* b 2))) (display (+ a b c)))) -> (let* ((a 1) (b (+ a 1)) (c (* b 2))) (display (+ a b c)))
				    (lint-format "perhaps ~A" caller
						 (lists->string form
								`(let* ,(append outer-vars inner-vars)
								   ,@(one-call-and-dots (cddr inner))))))
				   ((and (pair? (cddr inner))
					 (pair? (caddr inner))
					 (null? (cdddr inner))
					 (eq? (caaddr inner) 'let)
					 (pair? (cdr (caddr inner)))
					 (pair? (cadr (caddr inner))))
				    (let* ((inner1 (cdaddr inner))
					   (inner1-vars (car inner1)))
				      (if (and (pair? (cdr inner1))
					       (null? (cddr inner1))
					       (pair? (cadr inner1))
					       (eq? (caadr inner1) 'let)
					       (pair? (cdadr inner1))
					       (pair? (cadadr inner1)))
					  (let* ((inner2 (cdadr inner1))
						 (inner2-vars (car inner2)))
					    (if (not (letstar outer-vars
							      inner-vars
							      inner1-vars
							      inner2-vars))
						;; (let ((a 1)) (let ((b 2)) (let ((c 3)) (let ((d 4)) (+ a b c d))))) -> (let ((a 1) (b 2) (c 3) (d 4)) (+ a b c d))
						(lint-format "perhaps ~A" caller
							     (lists->string form
									    `(let ,(append outer-vars inner-vars inner1-vars inner2-vars)
									       ,@(one-call-and-dots (cdr inner2)))))))
					  (if (not (letstar outer-vars
							    inner-vars
							    inner1-vars))
					      ;; (let ((b 2)) (let ((c 3)) (let ((d 4)) (+ a b c d)))) -> (let ((b 2) (c 3) (d 4)) (+ a b c d))
					      (lint-format "perhaps ~A" caller
							   (lists->string form
									  `(let ,(append outer-vars inner-vars inner1-vars)
									     ,@(one-call-and-dots (cdr inner1)))))))))
				   ((not (letstar outer-vars
						  inner-vars))
				    ;; (let ((c 3)) (let ((d 4)) (+ a b c d))) -> (let ((c 3) (d 4)) (+ a b c d))
				    (lint-format "perhaps ~A" caller
						 (lists->string form
								`(let ,(append outer-vars inner-vars)
								   ,@(one-call-and-dots (cddr inner))))))
				   
				   ((and (null? (cdadr form))   ; 1 outer var
					 (pair? inner-vars)  
					 (null? (cdadr inner))) ; 1 inner var, dependent on outer
				    ;; (let ((x 0)) (let ((y (g 0))) (+ x y))) -> (let* ((x 0) (y (g 0))) (+ x y))
				    (lint-format "perhaps ~A" caller
						 (lists->string form
								`(let* ,(append outer-vars inner-vars)
								   ,@(one-call-and-dots (cddr inner))))))))))))
		   ))) ; messed up let
	   env)
	 (hash-table-set! h 'let let-walker))	
	
	;; ---------------- let* ----------------		  	
	(let ()
	 (define (let*-walker caller form env)
	   (if (< (length form) 3)
	       (lint-format "let* is messed up: ~A" caller (truncated-list->string form))
	       (let ((named-let (and (symbol? (cadr form)) (cadr form))))
		 
		 (let ((vars (if named-let (list (make-var :name named-let 
							   :definer 'let*)) ())) ; TODO: fvar
		       (varlist ((if named-let caddr cadr) form))
		       (body ((if named-let cdddr cddr) form)))
		   (if (not (list? varlist))
		       (lint-format "let* is messed up: ~A" caller (truncated-list->string form)))

		   ;; let->do (could go further down)
		   (when (and (integer? *max-cdr-len*)
			      (pair? varlist)
			      (pair? body)
			      (pair? (car body))
			      (eq? (caar body) 'do)
			      (< (tree-leaves (cdr body)) *max-cdr-len*))
		     (let ((inits (if (pair? (cadar body))
				      (map cadr (cadar body))
				      ()))
			   (locals (if (pair? (cadar body))
				       (map car (cadar body))
				       ()))
			   (lv (list-ref varlist (- (length varlist) 1))))
		       (unless (and (pair? inits)
				    (or (memq (car lv) locals) ; shadowing
					(tree-memq (car lv) inits)
					(side-effect? (cadr lv) env)))
			 ;; (let* ((x (log z))) (do ((i 0 (+ x z))) ((= i 3)) (display x))) -> (do ((x (log z)) (i 0 (+ x z))) ...)
			 (lint-format "perhaps ~A" caller
				      (lists->string form
						     (let ((new-do (let ((do-form (cdar body)))
								     (if (null? (cdr body))
									 `(do ,(cons lv (car do-form))
									      ...)
									 `(do ,(cons lv (car do-form))
									      (,(and (pair? (cadr do-form)) (caadr do-form))
									       ,@(if (side-effect? (cdadr do-form) env) (cdadr do-form) ())
									       ,@(cdr body))   ; include rest of let as do return value
									    ...)))))
						       (case (length varlist)
							 ((1) new-do)
							 ((2) `(let (,(car varlist)) ,new-do))
							 (else `(let* ,(copy varlist (make-list (- (length varlist) 1)))
								  ,new-do)))))))))
		   (do ((side-effects #f)
			(bindings varlist (cdr bindings)))
		       ((not (pair? bindings))
			(if (not (null? bindings))
			    (lint-format "let* variable list is not a proper list? ~S" 
					 caller ((if named-let caddr cadr) form)))
			(if (not (or side-effects
				     (any? (lambda (v) (positive? (var-ref v))) vars)))
			    ;; (let* ((x (log y))) x)
			    (lint-format "let* could be let: ~A" caller (truncated-list->string form))))
		     ;; in s7, let evaluates var values top down, so this message is correct
		     ;;   even in cases like (let ((ind (open-sound...)) (mx (maxamp))) ...)
		     ;; in r7rs, the order is not specified (section 4.2.2 of the spec), so
		     ;;   here we would restrict this message to cases where there is only
		     ;;   one variable, or where subsequent values are known to be independent.
		     ;; if each function could tell us what globals it depends on or affects,
		     ;;   we could make this work in all cases.
		     
		     (when (binding-ok? caller 'let* (car bindings) env #f)
		       (let ((expr (cadar bindings))
			     (side (side-effect? (cadar bindings) env)))
			 (if (not (or (eq? bindings varlist)
				      ;; first var side-effect is innocuous (especially if it's the only one!)
				      ;;    does this need to protect against a side-effect that the next var accesses?
				      ;;    I think we're ok -- the accessed var must be exterior, and we go down in order
				      side-effects))
			     (set! side-effects side))
			 (lint-walk caller expr (append vars env))
			 (set! vars (cons (make-var :name (caar bindings) 
						    :initial-value expr
						    :definer (if named-let 'named-let* 'let*))
					  vars))
			 
			 ;; look for duplicate values
			 ;; TODO: protect against any shadows if included in any expr
			 (if (not (or side 
				      (not (pair? expr))
				      (code-constant? expr)
				      (maker? expr)))
			     (let ((name (caar bindings)))
			       (let dup-check ((vs (cdr vars)))
				 (if (and (pair? vs)
					  (pair? (car vs))
					  (not (eq? name (caar vs)))
					  (not (tree-memq (caar vs) expr)))
				     ;; perhaps also not side-effect of car vs initial-value (char-ready? + read + char-ready? again)
				     (if (equal? expr (var-initial-value (car vs)))
					 ;; (let* ((x (log y 2)) (y (log y 2)) (z (f x))) (+ x y z z))
					 (lint-format "~A's value ~A could be ~A" caller
						      name expr (caar vs))
					 (dup-check (cdr vs))))))))))

		   ;; if var is not used except in other var bindings, it can be moved out of this let*
		   ;;   collect vars not in body, used in only one binding, gather these cases, and rewrite the let*
		   ;;   repeated names are possible here
		   ;;   also cascading dependencies: (let* ((z 1) (y (+ z 2)) (x (< y 3))) (if x (f x)))
		   ;;                                (let ((x (let ((y (let ((z 1))) (+ z 2))) (< y 3)))) ...) ??
		   ;;                      new-vars: ((z y) (y x))
		   (when (and (pair? vars)
			      (pair? (cdr vars)))
		     (let ((new-vars ())
			   (vs-pos vars)
			   (repeats (do ((p vars (cdr p)))
					((or (null? p)
					     (var-member (var-name (car p)) (cdr p)))
					 (pair? p)))))
		       (for-each (lambda (v)
				   (let ((vname (var-name v))
					 (vvalue #f))
				     (if (not (tree-memq vname body))
					 (let walker ((vs vars))
					   (if (not (pair? vs))
					       (if (and vvalue
							(or (not (side-effect? (var-initial-value v) env))
							    (eq? vvalue (var-name (car vs-pos)))))
						   (set! new-vars (cons (list vvalue vname (var-initial-value v)) new-vars)))
					       (let ((b (car vs)))
						 (if (or (eq? (var-name b) vname)
							 (not (tree-memq vname (var-initial-value b)))) ; tree-memq matches the bare symbol (tree-member doesn't)
						     (walker (cdr vs))
						     (if (not vvalue)
							 (begin
							   (set! vvalue (var-name b))
							   (walker (cdr vs)))))))))
				     (set! vs-pos (cdr vs-pos))))
				 (cdr vars)) ; vars is reversed from code order, new-vars is in code order
		       
		       (when (pair? new-vars)
			 (define (gather-dependencies var val env)
			   (let ((deps ()))
			     (for-each (lambda (nv)
					 (if (and (eq? (car nv) var)
						  (or (not repeats)
						      (tree-memq (cadr nv) val)))
					     (set! deps (cons (list (cadr nv) 
								    (gather-dependencies (cadr nv) (caddr nv) env))
							      deps))))
				       new-vars)
			     (if (> (tree-leaves val) 30)
				 (set! val '...))
			     (if (pair? deps)
				 `(,(if (null? (cdr deps)) 'let 'let*)
				   ,deps ,val)
				 val)))

			 (let ((new-let-binds (map (lambda (v)
						     (if (member (var-name v) new-vars (lambda (name lst) (eq? name (cadr lst))))
							 (values)
							 `(,(var-name v) ,(gather-dependencies (var-name v) (var-initial-value v) env))))
						   (reverse vars))))
			   ;; (let* ((a 1) (b 2) (c (+ a 1))) (* c 2)) -> (let* ((b 2) (c (let ((a 1)) (+ a 1)))) ...)
			   (lint-format "perhaps restrict ~{~A~^, ~} which ~A not used in the let* body ~A" caller
					(map cadr new-vars)
					(if (null? (cdr new-vars)) "is" "are")
					(lists->string form
						       `(,(if (null? (cdr new-let-binds))
							      'let 'let*)
							 ,new-let-binds
							 ...)))))

		       ;; this could be folded into the for-each above
		       (unless repeats
			 (let ((outer-vars ())
			       (inner-vars ()))
			   (do ((vs (reverse vars) (cdr vs)))
			       ((null? vs))
			     (let* ((v (car vs))
				    (vname (var-name v)))
			       
			       (if (not (or (side-effect? (var-initial-value v) env)
					    (any? (lambda (trailing-var)
						    ;; vname is possible inner let var if it is not mentioned in any trailing initial value
						    ;;   (repeated name can't happen here)
						    (tree-memq vname (var-initial-value trailing-var)))
						  (cdr vs))))
				   (set! inner-vars (cons v inner-vars))
				   (set! outer-vars (cons v outer-vars)))))
			   (if (and (pair? outer-vars)
				    (pair? inner-vars)
				    (pair? (cdr inner-vars)))
			       ;; (let* ((a 1) (b 2) (c (+ a 1))) (* c 2)) -> (let ((a 1)) (let ((b 2) (c (+ a 1))) ...))
			       (lint-format "perhaps split this let*: ~A" caller
					    (lists->string form
							   `(,(if (pair? (cdr outer-vars)) 'let* 'let)
							     ,(map (lambda (v)
								     `(,(var-name v) ,(var-initial-value v)))
								   (reverse outer-vars))
							     (let ,(map (lambda (v)
									  `(,(var-name v) ,(var-initial-value v)))
									(reverse inner-vars))
							       ...)))))))
			 )) ; pair? vars
		   
		   (let* ((cur-env (cons (make-var :name :let
						   :initial-value form
						   :definer 'let*)
					 (append vars env)))
			  (e (lint-walk-body caller 'let* body cur-env)))

		     (let ((nvars (and (not (eq? e cur-env))
				       (env-difference caller e cur-env ()))))
		       (if (pair? nvars)
			   (if (memq (var-name (car nvars)) '(:lambda :dilambda))
			       (begin
				 (set! env (cons (car nvars) env))
				 (set! nvars (cdr nvars)))
			       (set! vars (append nvars vars)))))
		     
		     (report-usage caller 'let* vars cur-env))
		   
		   (when (and (not named-let)
			      (pair? body)
			      (pair? varlist)) ; from here to end
		     
		     ;; (let*->let*) combined into one
		     (when (and (pair? (car body))
				(or (eq? (caar body) 'let*)      ; let*+let* -> let*
				    (and (eq? (caar body) 'let)  ; let*+let(1) -> let*
					 (or (null? (cadar body))
					     (and (pair? (cadar body))
						  (null? (cdadar body))))))
				(null? (cdr body))
				(not (symbol? (cadar body))))
		       ;; (let* ((a 1) (b (+ a 2))) (let* ((c (+ b 3)) (d (+ c 4))) (display a) (+ a... ->
		       ;;    (let* ((a 1) (b (+ a 2)) (c (+ b 3)) (d (+ c 4))) (display a) ...)
		       (lint-format "perhaps ~A" caller
				    (lists->string form
						   `(let* ,(append varlist (cadar body))
						      ,@(one-call-and-dots (cddar body))))))
		     
		     (when (and (proper-list? (cadr form))
				(not (tree-set-member '(curlet lambda lambda* define define*) (cddr form))))
		       ;; see let above
		       (do ((changes ())
			    (vs (cadr form) (cdr vs)))
			   ((null? vs)
			    (if (pair? changes)
				(let ((new-form (copy form)))
				  (for-each 
				   (lambda (v)
				     (list-set! new-form 1 (remove-if (lambda (p) (equal? p v)) (cadr new-form)))
				     (set! new-form (tree-subst (cadr v) (car v) new-form)))
				   changes)
				  ;; (let* ((x y) (a (* 2 x))) (+ (f a (+ a 1)) (* 3 x))) -> (let ((a (* 2 y))) (+ (f a (+ a 1)) (* 3 y)))
				  (lint-format "assuming we see all set!s, the binding~A ~{~A~^, ~} ~A pointless: perhaps ~A" caller
					       (if (pair? (cdr changes)) "s" "")
					       changes 
					       (if (pair? (cdr changes)) "are" "is")
					       (lists->string form
							      (let ((header (if (and (pair? (cadr new-form))
										     (pair? (cdadr new-form)))
										'let* 'let)))
								(if (< (tree-leaves new-form) 200)
								    `(,header ,@(cdr new-form))
								    `(,header ,(cadr new-form)
									      ,@(one-call-and-dots (cddr new-form))))))))))
			 (let ((v (car vs)))
			   (if (and (pair? v)
				    (pair? (cdr v))
				    (null? (cddr v))                  
				    (symbol? (cadr v))
				    (not (assq (cadr v) (cadr form))) ; value is not a local var
				    (not (set-target (car v) body env))
				    (not (set-target (cadr v) body env)))
			       (let ((data (var-member (cadr v) env)))
				 (if (and (or (not (var? data))
					      (and (not (eq? (var-definer data) 'parameter))
						   (or (null? (var-setters data))
						       (not (tree-set-member (var-setters data) body)))))
					  (not (any? (lambda (p)
						       (and (pair? p)
							    (pair? (cdr p))
							    (or (set-target (cadr v) (cdr p) env)
								(set-target (car v) (cdr p) env)
								(and (var? data)
								     (pair? (var-setters data))
								     (tree-set-member (var-setters data) body)))))
						     (cdr vs))))
				     (set! changes (cons v changes))))))))
		     
		     (let* ((varlist-len (length varlist))
			    (last-var (and (positive? varlist-len)
					   (list-ref varlist (- varlist-len 1))))) ; from here to end
		       (when (pair? last-var)  ; successive vars, first used in second but nowhere else -- combine if (very!) simple-looking
			 (do ((gone-vars ())
			      (v varlist (cdr v)))
			     ((or (null? v)
				  (null? (cdr v)))
			      
			      (when (pair? gone-vars)
				(let ((waiter #f)
				      (new-vars ())
				      (save-vars ()))
				  (set! gone-vars (reverse gone-vars))
				  (set! new-vars (map (lambda (v)
							(if (and (pair? gone-vars)
								 (eq? v (car gone-vars)))
							    (begin
							      (set! waiter v)
							      (set! gone-vars (cdr gone-vars))
							      (values))
							    (if (not waiter)
								v
								(let ((new-v (tree-subst (cadr waiter) (car waiter) v)))
								  (set! save-vars (cons (list (car waiter) (car v)) save-vars))
								  (set! waiter #f)
								  new-v))))
						      varlist))
				  ;; (let* ((y 3) (x (log y))) x) -> (let ((x (log 3))) ...)
				  (lint-format "perhaps substitute ~{~{~A into ~A~}~^, ~}: ~A" caller 
					       (reverse save-vars)
					       (lists->string form
							      `(,(if (null? (cdr new-vars)) 'let 'let*)
								,new-vars
								...))))))
			   (let ((cur-var (car v))
				 (nxt-var (cadr v)))
			     (when (and (pair? cur-var)
					(pair? nxt-var)
					(pair? (cdr cur-var))
					(pair? (cdr nxt-var))
					(< (tree-leaves (cadr cur-var)) 8)
					(not (and (pair? (cadr nxt-var))
						  (eq? (caadr nxt-var) 'let) ; if named-let, forget it
						  (pair? (cdadr nxt-var))
						  (symbol? (cadadr nxt-var))))
					(or (not (pair? (cadr nxt-var)))
					    (not (side-effect? (cadr cur-var) env))
					    (every? (lambda (a)
						      (or (code-constant? a)
							  (assq a varlist)))
						    (cdadr nxt-var)))
					(= (tree-count1 (car cur-var) (cadr nxt-var) 0) 1)
					(not (tree-memq (car cur-var) (cddr v)))
					(not (tree-memq (car cur-var) body)))
			       (set! gone-vars (cons cur-var gone-vars))
			       (set! v (cdr v)))))
			 
			 ;; if last var only occurs once in body, and timing can't be an issue, substitute its value
			 ;;   this largely copied from the let case above (but only one substitution)
			 ;;   in both cases, we're assuming that the possible last-var value's side-effect won't
			 ;;      affect other vars (in let* the local, in let something outside that might be used locally)
			 ;;      perhaps add (not (side-effect (cadr last-var) env))?
			 
			 (when (and (pair? (cdr last-var))  ; varlist-len can be 1 here
				    (< (tree-leaves (cadr last-var)) 12)
				    (= (tree-count1 (car last-var) body 0) 1)
				    (pair? (car body))
				    (null? (cdr body))
				    (not (memq (caar body) '(lambda lambda* define define* define-macro)))
				    (not (and (eq? (caar body) 'set!)
					      (eq? (car last-var) (cadar body))))
				    (not (any-macro? (caar body) env))
				    (not (any? (lambda (p)
						 (and (pair? p)
						      (not (eq? (car p) 'quote))
						      (or (not (hash-table-ref no-side-effect-functions (car p)))
							  (any? pair? (cdr p)))))
					       (cdar body))))
			   ;; (let* ((a 1) (b 2) (c (+ a 1))) (* c 2)) -> (let* ((a 1) (b 2)) (* (+ a 1) 2))
			   (lint-format "perhaps ~A" caller 
					(lists->string form `(,(if (<= varlist-len 2) 'let 'let*)
							      ,(copy varlist (make-list (- varlist-len 1)))
							      ,@(tree-subst (cadr last-var) (car last-var) body)))))
			 
			 (when (null? (cdr body)) ; (let* (...(x A)) (if x (f A) B)) -> (let(*) (...) (cond (A => f) (else B)))
			   
			   (when (pair? (cdr last-var))
			     (let ((p (car body)))
			       (when (and (pair? p)
					  (pair? (cdr p))
					  (case (car p)
					    ((if and) (eq? (cadr p) (car last-var)))
					    ((or)     (equal? (cadr p) `(not ,(car last-var))))
					    (else #f))
					  (pair? (cddr p))
					  (pair? (caddr p))
					  (or (eq? (car p) 'if)
					      (null? (cdddr p)))
					  (pair? (cdaddr p))
					  (not (eq? (caaddr p) (car last-var))) ; ! (let* (...(x A)) (if x (x x)))
					  (null? (cddr (caddr p)))
					  (eq? (car last-var) (cadr (caddr p))))
				 
				 (let ((else-clause (if (pair? (cdddr p)) ; only if 'if (see above)
							(if (eq? (cadddr p) (car last-var))
							    `((else #f)) ; this stands in for the local var
							    (if (and (pair? (cadddr p))
								     (tree-unquoted-member (car last-var) (cadddr p)))
								:oops! ; if the let var appears in the else portion, we can't do anything with =>
								`((else ,(cadddr p)))))
							(case (car p)
							  ((and) '((else #f)))
							  ((or)  '((else #t)))
							  (else  ())))))
				   (if (not (eq? else-clause :oops!))
				       ;; (let* ((x (f y))) (and x (g x))) -> (cond ((f y) => g) (else #f)
				       (lint-format "perhaps ~A" caller
						    (case varlist-len
						      ((1) (lists->string form 
									  `(cond (,(cadr last-var) => ,(caaddr p)) ,@else-clause)))
						      ((2) (lists->string form 
									  `(let (,(car varlist))
									     (cond (,(cadr last-var) => ,(caaddr p)) ,@else-clause))))
						      (else (lists->string form 
									   `(let* ,(copy varlist (make-list (- varlist-len 1)))
									      (cond (,(cadr last-var) => ,(caaddr p)) ,@else-clause)))))))))))
			   
			   (when (and (pair? (car varlist))      ; same as let: (let* ((x y)) x) -> y -- (let* (x) ...)
				      (not (pair? (car body))))
			     (if (and (eq? (car body) (caar varlist))
				      (null? (cdr varlist))
				      (pair? (cdar varlist))) ; (let* ((a...)) a)
				 ;; (let* ((x (log y))) x) -> (log y)
				 (lint-format "perhaps ~A" caller (lists->string form (cadar varlist)))
				 (if (and (> varlist-len 1)         ; (let* (... (x y)) x) -> (let(*)(...) y)
					  (pair? last-var)
					  (pair? (cdr last-var))
					  (null? (cddr last-var))
					  (eq? (car body) (car last-var)))
				     ;; (let* ((y 3) (x (log y))) x) -> (let ((y 3)) (log y))
				     (lint-format "perhaps ~A" caller 
						  (lists->string form `(,(if (= varlist-len 2) 'let 'let*)
									,(copy varlist (make-list (- varlist-len 1)))
									,(cadr last-var)))))))))
		       (when (and (> (length body) 3)
				  (> (length vars) 1)
				  (every? pair? varlist)
				  (not (tree-set-car-member '(define define* define-macro define-macro* 
							       define-bacro define-bacro* define-constant define-expansion)
							    body)))
			 (let ((last-ref (vector (var-name (car vars)) #f 0 (car vars))))
			   (do ((p body (cdr p))
				(i 0 (+ i 1)))
			       ((null? p)
				(let ((cur-line (last-ref 1))
				      (max-line (last-ref 2))
				      (vname (last-ref 0)))
				  (if (and (< max-line (/ i lint-let-reduction-factor))
					   (> (- i max-line) 3))
				      (lint-format "the scope of ~A could be reduced: ~A" caller 
						   vname
						   (lists->string form
								  `(,(if (> (length vars) 2) 'let* 'let)
								    ,(copy varlist (make-list (- (length vars) 1)))
								    (let (,(list vname (var-initial-value (last-ref 3))))
								      ,@(copy body (make-list (+ max-line 1))))
								    ,(list-ref body (+ max-line 1))
								    ...)))
				      (if (and (integer? cur-line)
					       (< (- max-line cur-line) 2)
					       (code-constant? (var-initial-value (last-ref 3))))
					  (lint-format "~A is only used in expression~A (of ~A),~%~NC~A~A of~%~NC~A" caller
						       vname
						       (if (= cur-line max-line)
							   (format #f " ~D" (+ cur-line 1))
							   (format #f "s ~D and ~D" (+ cur-line 1) (+ max-line 1)))
						       (length body)
						       (+ lint-left-margin 6) #\space
						       (truncated-list->string (list-ref body cur-line))
						       (if (= cur-line max-line)
							   ""
							   (format #f "~%~NC~A" 
								   (+ lint-left-margin 6) #\space
								   (truncated-list->string (list-ref body max-line))))
						       (+ lint-left-margin 4) #\space
						       (truncated-list->string form))))))
			     (when (tree-memq (last-ref 0) (car p))
			       (set! (last-ref 2) i)
			       (if (not (last-ref 1)) (set! (last-ref 1) i))))))
		       )))))
	   env)
	 (hash-table-set! h 'let* let*-walker))		
	
	;; ---------------- letrec ----------------		  
	(let ()
	  (define (letrec-walker caller form env)
	    (if (< (length form) 3)                 ;  (letrec () . 1)
		(lint-format "~A is messed up: ~A" caller (car form) (truncated-list->string form))
		(let ((vars ())
		      (head (car form)))
		  
		  (cond ((null? (cadr form))        ;  (letrec () 1)
			 (lint-format "~A could be let: ~A" caller head (truncated-list->string form)))
			((not (pair? (cadr form)))  ;  (letrec a b)
			 (lint-format "~A is messed up: ~A" caller head (truncated-list->string form)))
			((and (null? (cdadr form))
			      (eq? head 'letrec*))  ;  (letrec* ((a (lambda b (a 1)))) a)
			 (lint-format "letrec* could be letrec: ~A" caller (truncated-list->string form))))
		  
		  (do ((warned (or (eq? head 'letrec*)
				   (not (pair? (cadr form)))
				   (negative? (length (cadr form))))) ; malformed letrec
		       (baddy #f)
		       (bindings (cadr form) (cdr bindings)))
		      ((not (pair? bindings))
		       (if (not (null? bindings)) ; (letrec* letrec)!
			   (lint-format "~A variable list is not a proper list? ~S" caller head (cadr form))))
		    
		    (when (and (not warned)              ; letrec -> letrec*
			       (pair? (car bindings))
			       (pair? (cdar bindings))
			       ;; type of current var is not important -- if used in non-function elsewhere,
			       ;;   it has to be letrec*
			       (any? (lambda (b)
				       (and (pair? b)
					    (pair? (cdr b))
					    (or (and (not (pair? (cadr b)))
						     (eq? (caar bindings) (cadr b)))
						(tree-memq (caar bindings) (cadr b)))
					    (not (tree-set-member '(lambda lambda* define define* case-lambda) (cadr b)))
					    (set! baddy b)))
				     (cdr bindings)))
		      (set! warned #t)
		      ;; (letrec ((x 32) (f1 (let ((y 1)) (lambda (z) (+ x y z)))) (f2 (f1 x))) (+ x f2))
		      (lint-format "in ~A,~%~NCletrec should be letrec* because ~A is used in ~A's value (not a function): ~A" caller
				   (truncated-list->string form)
				   (+ lint-left-margin 4) #\space
				   (caar bindings)
				   (car baddy)
				   (cadr baddy)))
		    
		    (when (binding-ok? caller head (car bindings) env #f)
		      (let ((init (if (and (eq? (caar bindings) (cadar bindings))
					   (or (eq? head 'letrec)
					       (not (var-member (caar bindings) vars))))
				      (begin ; (letrec ((x x)) x)
					(lint-format "~A is the same as (~A #<undefined>) in ~A" caller
						     (car bindings) (caar bindings) head)
					;; in letrec* ((x 12) (x x)) is an error
					#<undefined>)
				      (cadar bindings))))
			(set! vars (cons (make-var :name (caar bindings) 
						   :initial-value init
						   :definer head)
					 vars)))))
		  
		  (when (eq? head 'letrec)
		    (check-unordered-exprs caller form (map var-initial-value vars) env))
		  
		  (when (pair? vars)
		    (do ((bindings (cadr form) (cdr bindings)) ; if none of the local vars occurs in any of the values, no need for the "rec"
			 (vs (map var-name vars)))
			((or (not (pair? bindings))
			     (not (pair? (car bindings)))
			     (not (pair? (cdar bindings)))
			     (memq (cadar bindings) vs)
			     (tree-set-member vs (cadar bindings)))
			 (if (null? bindings)
			     (let ((letx (if (or (eq? head 'letrec)
						 (do ((p (map cadr (cadr form)) (cdr p))
						      (q (map car (cadr form)) (cdr q)))
						     ((or (null? p)
							  (side-effect? (car p) env)
							  (memq (car q) (cdr q)))
						      (null? p))))
					     'let 'let*)))
			       ;; (letrec ((f1 (lambda (a) a))) 32)
			       (lint-format "~A could be ~A: ~A" caller
					    head letx
					    (truncated-list->string form))))))
		    
		    (when (and (null? (cdr vars))
			       (pair? (cddr form))
			       (pair? (caddr form))
			       (null? (cdddr form)))
		      (let ((body (caddr form))
			    (sym (var-name (car vars)))
			    (lform (cadar (cadr form))))           ; the letrec var's lambda
			(when (and (pair? lform)
				   (pair? (cdr lform))
				   (eq? (car lform) 'lambda)
				   (proper-list? (cadr lform))) ; includes ()
			  (if (eq? sym (car body))                   ; (letrec ((x (lambda ...))) (x...)) -> (let x (...)...)
			      (if (and (not (tree-memq sym (cdr body)))
				       (< (tree-leaves body) 100))
				  ;; the limit on tree-leaves is for cases where the args are long lists of data --
				  ;;   more like for-each than let, and easier to read if the code is first, I think.
				  (lint-format "perhaps ~A" caller
					       (lists->string 
						form `(let ,sym 
							,(map list (cadr lform) (cdr body))
							,@(cddr lform)))))
			      (if (and (not (eq? caller 'define))
				       (= (tree-count1 sym body 0) 1))
				  (let ((call (find-call sym body)))
				    (when (pair? call)
				      (let ((new-call `(let ,sym
							 ,(map list (cadr lform) (cdr call))
							 ,@(cddr lform))))
					(lint-format "perhaps ~A" caller
						     (lists->string form (tree-subst new-call call body))))))))))))
		  ;; maybe (let () ...) here because (letrec ((x (lambda (y) (+ y 1)))) (x (define z 32))) needs to block z?
		  ;;   currently we get (let x ((y (define z 32))) (+ y 1))
		  ;;   and even that should be (let () (define z 32) (+ z 1)) or something similar
		  ;; lambda here is handled under define??
		  
		  (let ((new-env (append vars env)))
		    (when (pair? (cadr form))
		      (for-each (lambda (binding)
				  (if (binding-ok? caller head binding env #t)
				      (lint-walk caller (cadr binding) new-env)))
				(cadr form)))
		    
		    (let* ((cur-env (cons (make-var :name :let
						    :initial-value form
						    :definer head)
					  (append vars env)))
			   (e (lint-walk-body caller head (cddr form) cur-env)))

		      (let ((nvars (and (not (eq? e cur-env))
					(env-difference caller e cur-env ()))))
			(if (pair? nvars)
			    (if (memq (var-name (car nvars)) '(:lambda :dilambda))
				(begin
				  (set! env (cons (car nvars) env))
				  (set! nvars (cdr nvars)))
				(set! vars (append nvars vars)))))
		      
		      (report-usage caller head vars cur-env))))) ; constant exprs never happen here
	    env)
	  (hash-table-set! h 'letrec letrec-walker)
	  (hash-table-set! h 'letrec* letrec-walker))
	
	
	;; ---------------- begin ----------------
	(let ()
	 (define (begin-walker caller form env)
	   
	   (if (not (proper-list? form))
	       (begin                           ;  (begin . 1)
		 (lint-format "stray dot in begin? ~A" caller (truncated-list->string form))
		 env)
	       (begin
		 (if (and (pair? (cdr form))
			  (null? (cddr form)))  ;  (begin (f y))
		     (lint-format "begin could be omitted: ~A" caller (truncated-list->string form)))
		 (lint-walk-open-body caller 'begin (cdr form) env))))
	 (hash-table-set! h 'begin begin-walker))
	
	
	;; ---------------- with-baffle ----------------
	(let ()
	  (define (with-baffle-walker caller form env)
	    ;; with-baffle introduces a new frame, so we need to handle it here
	    (lint-walk-body caller 'with-baffle (cdr form) 
			    (cons (make-var :name :let
					    :initial-value form
					    :definer 'with-baffle)
				  env))
	    env)
	  (hash-table-set! h 'with-baffle with-baffle-walker))
	
	
	;; -------- with-let --------
	(let ()
	  (define (with-let-walker caller form env)
	    (if (< (length form) 3)
		(lint-format "~A is messed up: ~A" 'with-let caller (truncated-list->string form))
		(let ((e (cadr form)))
		  (if (or (and (code-constant? e)
			       (not (let? e)))
			  (and (pair? e)
			       (let ((op (return-type (car e) env)))
				 (and op
				      (not (return-type-ok? 'let? op))))))  ;  (with-let 123 123)
		      (lint-format "~A: first argument should be an environment: ~A" 'with-let caller (truncated-list->string form)))
		  
		  (if (symbol? e)
		      (set-ref e caller form env)
		      (if (pair? e)
			  (begin
			    (if (and (null? (cdr e))
				     (eq? (car e) 'curlet))    ;  (with-let (curlet) x)
				(lint-format "~A is not needed here: ~A" 'with-let caller (truncated-list->string form)))
			    (lint-walk caller e (cons (make-var :name :let
								:initial-value form
								:definer 'with-let)
						      env)))))
		  (let ((walked #f)
			(new-env (cons (make-var :name :with-let :initial-value form :definer 'with-let) env)))
		    (if (or (and (symbol? e)
				 (memq e '(*gtk* *motif* *gl* *libc* *libm* *libgdbm* *libgsl*)))
			    (and (pair? e)
				 (eq? (car e) 'sublet)
				 (pair? (cdr e))
				 (memq (cadr e) '(*gtk* *motif* *gl* *libc* *libm* *libgdbm* *libgsl*))
				 (set! e (cadr e))))
			(let ((lib (if (defined? e)
				       (symbol->value e)
				       (let ((file (*autoload* e)))
					 (and (string? file) 
					      (load file))))))
			  (when (let? lib)
			    (let-temporarily ((*e* lib))
			      (let ((e (lint-walk-open-body caller 'with-let (cddr form) new-env)))
				(report-usage caller 'with-let 
					      (if (eq? e env) 
						  () 
						  (env-difference caller e env ())) 
					      new-env)))
			    (set! walked #t))))
		    
		    (unless walked
		      (lint-walk-open-body caller 'with-let (cddr form) new-env)))))
	    env)
	  (hash-table-set! h 'with-let with-let-walker))
	
	
	;; ---------------- defmacro ----------------
	(let ()
	  (define (defmacro-walker caller form env)
	    (if (or (< (length form) 4)
		    (not (symbol? (cadr form))))
		(begin
		  (lint-format "~A declaration is messed up: ~A" caller (car form) (truncated-list->string form))
		  env)
		(let ((sym (cadr form))
		      (args (caddr form))
		      (body (cdddr form))
		      (head (car form)))
		  (if (and (pair? args)
			   (repeated-member? args env))                        ; (defmacro hi (a b a) a)
		      (lint-format "~A parameter is repeated: ~A" caller head (truncated-list->string args))
		      (lint-format "~A is deprecated; perhaps ~A" caller head  ; (defmacro hi (a b) `(+ ,a ,b))
				   (truncated-lists->string form 
							    `(,(if (eq? head 'defmacro) 'define-macro 'define-macro*) 
							      ,(cons sym args) 
							      ,@body))))
		  (lint-walk-function head sym args body form env)
		  (cons (make-var :name sym :initial-value form :definer head) env))))
	  (hash-table-set! h 'defmacro defmacro-walker)
	  (hash-table-set! h 'defmacro* defmacro-walker))
	
	
	;; ---------------- load ----------------
	(let ()
	  (define (load-walker caller form env)
	    (lint-walk caller (cdr form) env)
	    (if (and *report-loaded-files*
		     (string? (cadr form)))
		(catch #t
		  (lambda ()
		    (lint-file (cadr form) env))
		  (lambda args
		    env))
		env))
	  (hash-table-set! h 'load load-walker))
	
	
	;; ---------------- require ----------------
	(let ()
	  (define (require-walker caller form env)
	    (if (not (pair? (cdr form)))                   ; (require)
		(lint-format "~A is pointless" caller form) 
		(if (any? string? (cdr form))              ; (require "repl.scm")
		    (lint-format "in s7, require's arguments should be symbols: ~A" caller (truncated-list->string form))))
	    (if (not *report-loaded-files*)
		env
		(let ((vars env))
		  (for-each 
		   (lambda (f)
		     (let ((file (*autoload* f)))
		       (if (string? file)
			   (catch #t
			     (lambda ()
			       (set! vars (lint-file file vars)))
			     (lambda args
			       #f)))))
		   (cdr form))
		  vars)))
	  (hash-table-set! h 'require require-walker))

	
	;; ---------------- call-with-input-file etc ----------------
	(let ()
	  (define (call-with-io-walker caller form env)
	    (let ((len (if (eq? (car form) 'call-with-output-string) 2 3))) ; call-with-output-string func is the first arg, not second
	      (when (= (length form) len)
		(let ((func (list-ref form (- len 1))))
		  (if (= len 3)
		      (lint-walk caller (cadr form) env))
		  (if (not (and (pair? func)
				(eq? (car func) 'lambda)))
		      (lint-walk caller func env)
		      (let ((args (cadr func)))
			(let ((body (cddr func))
			      (port (and (pair? args) (car args)))
			      (head (car form)))
			  (if (or (not port)
				  (pair? (cdr args)))
			      ;; (lambda () (write args) (newline))
			      (lint-format "~A argument should be a function of one argument: ~A" caller head func)
			      (if (and (null? (cdr body))
				       (pair? (car body))
				       (pair? (cdar body))
				       (eq? (cadar body) port)
				       (null? (cddar body)))
				  ;; (call-with-input-file "file" (lambda (p) (read-char p))) -> (call-with-input-file "file" read-char)
				  (lint-format "perhaps ~A" caller 
					       (lists->string form 
							      (if (= len 2)
								  `(,head ,(caar body))
								  `(,head ,(cadr form) ,(caar body)))))
				  (let ((cc (make-var :name port
						      :initial-value (list (case head 
									     ((call-with-input-string)  'open-input-string)
									     ((call-with-output-string) 'open-output-string)
									     ((call-with-input-file)    'open-input-file)
									     ((call-with-output-file)   'open-output-file)))
						      :definer head)))
				    (lint-walk-body caller head body (cons cc 
									   (cons (make-var :name :let
											   :initial-value form
											   :definer head)
										 env)))
				    (report-usage caller head (list cc) env))))))))))
	    env)
	  (for-each (lambda (op)
		      (hash-table-set! h op call-with-io-walker))
		    '(call-with-input-string call-with-input-file call-with-output-file call-with-output-string)))
	
	
	;; ---------------- catch ----------------
	(let ()
	 (define (catch-walker caller form env)
	   ;; catch tag is tricky -- it is evaluated, then eq? matches at error time, so we need
	   ;;   to catch constants that can't be eq?
	   (if (not (= (length form) 4))
	       (begin
		 (lint-format "catch takes 3 arguments (tag body error-handler): ~A" caller (truncated-list->string form))
		 (lint-walk caller (cdr form) env))
	       (let ((tag (cadr form)))
		 (if (or (and (not (pair? tag))
			      (or (number? tag) (char? tag) (length tag)))
			 (and (pair? tag)
			      (eq? (car tag) 'quote)
			      (or (not (pair? (cdr tag)))
				  (length (cadr tag)))))
		     ;; (catch #(0) (lambda () #f) (lambda a a))
		     (lint-format "catch tag ~S is unreliable (catch uses eq? to match tags)" caller tag))
		 (let ((body (caddr form))
		       (error-handler (cadddr form)))
		   ;; empty catch+catch apparently never happens
		   (lint-walk caller body (cons (make-var :name :let
							  :initial-value form
							  :definer 'catch)
						(cons (make-var :name :catch
								:initial-value form
								:definer 'catch)
						      env)))
		   (lint-walk caller error-handler env))))
	   env)
	 (hash-table-set! h 'catch catch-walker))
	
	
	;; ---------------- call-with-exit etc ----------------
	(let ()
	  (define (call-with-exit-walker caller form env)
	    (let ((continuation (and (pair? (cdr form))
				     (pair? (cadr form))
				     (eq? (caadr form) 'lambda)
				     (pair? (cdadr form))
				     (pair? (cddadr form))
				     (pair? (cadadr form))
				     (car (cadadr form)))))
	      (if (not (symbol? continuation))
		  (lint-walk caller (cdr form) env)
		  (let ((body (cddadr form))
			(head (car form)))
		    
		    (if (not (or (eq? head 'call-with-exit)
				 (eq? continuation (car body))
				 (tree-sym-set-member continuation '(lambda lambda* define define* curlet error apply) body)))
			;; this checks for continuation as arg (of anything), and any of set as car
			;; (call/cc (lambda (p) (+ x (p 1))))
			(lint-format* caller
				      (string-append "perhaps " (symbol->string head))
				      " could be call-with-exit: " 
				      (truncated-list->string form)))
		    
		    (if (not (tree-unquoted-member continuation body))
			;; (call-with-exit (lambda (p) (+ x 1)))
			(lint-format "~A ~A ~A appears to be unused: ~A" caller head 
				     (if (eq? head 'call-with-exit) "exit function" "continuation")
				     continuation
				     (truncated-list->string form))
			(let ((last (and (proper-list? body)
					 (list-ref body (- (length body) 1)))))
			  (if (and (pair? last)
				   (eq? (car last) continuation))
			      ;; (call-with-exit (lambda (return) (display x) (return (+ x y))))
			      (lint-format "~A is redundant here: ~A" caller continuation (truncated-list->string last)))))
		    
		    (let ((cc (make-var :name continuation 
					:initial-value (if (eq? head 'call-with-exit) :call/exit :call/cc)
					:definer head)))
		      (lint-walk-body caller head body (cons cc env))
		      (report-usage caller head (list cc) env)))))
	    env)
	  (for-each (lambda (op)
		      (hash-table-set! h op call-with-exit-walker))
		    '(call/cc call-with-current-continuation call-with-exit)))
	
	
	;; ---------------- import etc ----------------
	(for-each (lambda (op)
		    (hash-table-set! h op (lambda (caller form env) env)))
		  '(define-module import export))
	
	(hash-table-set! 
	 h 'provide
	 (lambda (caller form env)
	   (if (not (= (length form) 2))
	       ;; (provide a b c)
	       (lint-format "provide takes one argument: ~A" caller (truncated-list->string form))
	       (unless (symbol? (cadr form))
		 (let ((op (->lint-type (cadr form))))
		   (if (not (memq op '(symbol? #f #t values)))
		       ;; (provide "test")
		       (lint-format "provide's argument should be a symbol: ~S" caller form)))))
	   env))
	
	(hash-table-set! 
	 h 'module        ; module apparently has different syntax and expectations in various schemes
	 (lambda (caller form env)
	   (if (and (pair? (cdr form))
		    (pair? (cddr form)))
	       (lint-walk 'module (cddr form) env))
	   env))
	
	(hash-table-set! 
	 h 'define-syntax
	 (lambda (caller form env)
	   ;; we need to put the macro name in env with ftype=define-syntax
	   (if (and (pair? (cdr form))
		    (symbol? (cadr form))
		    (not (keyword? (cadr form)))) ; !! this thing is a disaster from the very start
	       (cons (make-fvar (cadr form) :ftype 'define-syntax) env)
	       env)))
	
	(hash-table-set! 
	 h 'define-method   ; guile and mit-scheme have different syntaxes here
	 (lambda (caller form env)
	   (if (not (and (pair? (cdr form))
			 (pair? (cddr form))))
	       env
	       (if (symbol? (cadr form))
		   (if (keyword? (cadr form))
		       (lint-walk-body caller 'define-method (cdddr form) env)
		       (let ((new-env (if (var-member (cadr form) env)
					  env
					  (cons (make-fvar (cadr form) :ftype 'define-method) env))))
			 (lint-walk-body caller (cadr form) (cdddr form) new-env)))
		   (let ((new-env (if (var-member (caadr form) env)
				      env
				      (cons (make-fvar (caadr form) :ftype 'define-method) env))))
		     (lint-walk-body caller (caadr form) (cddr form) new-env))))))
	
	(hash-table-set! h 'let-syntax (lambda (caller form env)
					 (lint-walk-body caller 'define-method (cddr form) env)
					 env))
	
	(hash-table-set! h 'letrec-syntax (lambda (caller form env)
					    (lint-walk-body caller 'define-method (cddr form) env)
					    env))
	
	
	;; ---------------- case-lambda ----------------
	(let ()
	  (define (case-lambda-walker caller form env)
	    (when (pair? (cdr form))
	      (let ((lens ())
		    (body ((if (string? (cadr form)) cddr cdr) form)) ; might have a doc string before the clauses
		    (doc-string (and (string? (cadr form)) (cadr form))))
		
		(define (arg->defaults arg b1 b2 defaults)
		  (and defaults
		       (cond ((null? b1) (and (null? b2) defaults))
			     ((null? b2) (and (null? b1) defaults))
			     ((eq? arg b1) (cons b2 defaults))
			     ((eq? arg b2) (cons b1 defaults))
			     ((pair? b1)
			      (and (pair? b2)
				   (arg->defaults arg (car b1) (car b2) (arg->defaults arg (cdr b1) (cdr b2) defaults))))
			     (else (and (equal? b1 b2) defaults)))))
		(for-each 
		 (lambda (choice)
		   (if (pair? choice)
		       (let ((len (length (car choice))))
			 (if (member len lens)
			     ;; (case-lambda (() 0) ((x y) x) ((x y) (+ x y)) ((x y z) (+ x y z)) (args (apply + args))
			     (lint-format "repeated parameter list? ~A in ~A" caller (car choice) form))
			 (set! lens (cons len lens))
			 (lint-walk 'case-lambda (cons 'lambda choice) env))))
		 body)
		
		(case (length lens)
		  ((1) 
		   ;; (case-lambda (() (if #f #f))) -> (lambda () (if #f #f))
		   (lint-format "perhaps ~A" caller 
				(lists->string form 
					       (if doc-string
						   `(let ((documentation ,doc-string))
						      (lambda ,(caar body) ,@(cdar body)))
						   `(lambda ,(caar body) ,@(cdar body))))))
		  ((2) 
		   (when (let arglists-equal? ((args1 (caar body))
					       (args2 (caadr body)))
			   (if (null? args1)
			       (and (pair? args2) (null? (cdr args2)))
			       (and (pair? args1)
				    (if (null? args2)
					(null? (cdr args1))
					(and (pair? args2)
					     (eq? (car args1) (car args2))
					     (arglists-equal? (cdr args1) (cdr args2)))))))
		     (let* ((clause1 (car body))
			    (body1 (cdr clause1))
			    (clause2 (cadr body))
			    (body2 (cdr clause2))
			    (arglist (let ((arg1 (car clause1))
					   (arg2 (car clause2)))	  
				       (if (> (car lens) (cadr lens)) arg2 arg1))) ; lens is reversed
			    (arg-name (list-ref arglist (- (length arglist) 1)))
			    (diffs (arg->defaults arg-name body1 body2 ())))
		       (when (and (pair? diffs)
				  (null? (cdr diffs))
				  (code-constant? (car diffs)))
			 (let ((new-body (if (> (car lens) (cadr lens)) body2 body1))
			       (new-arglist (if (not (car diffs))
						arglist
						(if (null? (cdr arglist))
						    `((,arg-name ,(car diffs)))
						    `(,(car arglist) (,arg-name ,(car diffs)))))))
			   ;; (case-lambda (() (display x #f)) ((y) (display x y))) -> (lambda* (y) (display x y))
			   (lint-format "perhaps ~A" caller
					(lists->string form
						       (if doc-string
							   `(let ((documentation ,doc-string))
							      (lambda* ,new-arglist ,@new-body))
							   `(lambda* ,new-arglist ,@new-body))))))))))))
	    env)
	  (hash-table-set! h 'case-lambda case-lambda-walker))
	h))
    ;; end walker-functions
    ;; ----------------------------------------


    (define lint-walk-pair 
      (let ((unsafe-makers '(sublet inlet copy cons list append make-shared-vector vector hash-table hash-table* 
			     make-hash-table make-hook #_{list} #_{append} gentemp or and not))
	    (qq-form #f))
	(lambda (caller form env)
	  (let ((head (car form)))
	    (set! line-number (pair-line-number form))
	    
	    (when *report-function-stuff* 
	      (function-match caller form env))

	    ;; differ-in-one here across args gets few interesting hits

	    (cond 
	     ((hash-table-ref walker-functions head)
	      => (lambda (f)
		   (f caller form env)))
	     (else
	      (if (not (proper-list? form))
		  ;; these appear to be primarily macro/match arguments
		  ;; other cases (not list) have already been dealt with far above
		  (if (and (pair? form)
			   (symbol? head)
			   (procedure? (symbol->value head *e*)))
		      ;; (+ . 1)
		      (lint-format "unexpected dot: ~A" caller (truncated-list->string form)))
		  (begin
		    (cond ((symbol? head)
			   (let ((v (var-member head env)))
			     (if (and (var? v) 
				      (not (memq form (var-history v))))
				 (set! (var-history v) (cons form (var-history v))))
			     (check-call caller head form env)
			     
			     ;; look for one huge argument leaving lonely trailing arguments somewhere off the screen
			     ;;   (it needs to be one arg, not a call on values)
			     (let ((branches (length form)))

			       (when (and (= branches 2)
					  (any-procedure? head env)
					  (not (eq? head 'unquote)))
				 (let ((arg (cadr form)))
				   ;; begin=(car arg) happens very rarely
				   (when (pair? arg)
				     (when (and (memq (car arg) '(let let*))
						(not (or (symbol? (cadr arg))
							 (and (pair? (cddr arg))
							      (pair? (caddr arg))
							      (eq? 'lambda (caaddr arg)))
							 (assq head (cadr arg)))))
				       ;; (string->symbol (let ((s (copy vstr))) (set! (s (+ pos 1)) #\s) s)) ->
				       ;;    (let ((s (copy vstr))) (set! (s (+ pos 1)) #\s) (string->symbol s))")
				       (lint-format "perhaps~%~NC~A ->~%~NC~A" caller
						    (+ lint-left-margin 4) #\space
						    (truncated-list->string form)
						    (+ lint-left-margin 4) #\space
						    (let* ((body (cddr arg))
							   (len (- (length body) 1))
							   (str (object->string `(,(car arg) ,(cadr arg)
										  ,@(copy body (make-list len))
										  (,head ,(list-ref body len))))))
						      (if (<= (length str) target-line-length)
							  str
							  (format #f "(~A ... (~A ~A))"
								  (car arg) head
								  (truncated-list->string (list-ref body len)))))))
				     (when (eq? (car arg) 'or)
				       (let ((else-clause (let ((last-clause (list-ref arg (- (length arg) 1))))
							    (if (and (pair? last-clause)
								     (memq (car last-clause) '(error throw)))
								last-clause
								(if (or (not (code-constant? last-clause))
									(side-effect? `(,head ,last-clause) env))
								    :checked-eval-error
								    (let ((res (checked-eval `(,head ,last-clause))))
								      (if (or (and (symbol? res)
										   (not (eq? res :checked-eval-error)))
									      (pair? res))
									  (list 'quote res)
									  res)))))))
					 (unless (eq? else-clause :checked-eval-error)
					   (set! last-rewritten-internal-define form)
					   ;; (string->number (or (f x) "4")) -> (cond ((f x) => string->number) (else 4))
					   (lint-format "perhaps ~A" caller
							(lists->string form
								       `(cond (,(if (or (null? (cddr arg))
											(null? (cdddr arg)))
										    (cadr arg)
										    (copy arg (make-list (- (length arg) 1))))
									       => ,head)
									      (else ,else-clause))))))))))

			       (unless (or (<= branches 2)
					   (any-macro? head env)
					   (memq head '(for-each map #_{list} * + - /)))
				 (let ((leaves (tree-leaves form)))
				   (when (> leaves (max *report-bloated-arg* (* branches 3)))
				     (do ((p (cdr form) (cdr p))
					  (i 1 (+ i 1)))
					 ((or (not (pair? p))
					      (null? (cdr p))
					      (and (pair? (car p))
						   (symbol? (caar p))
						   (not (memq (caar p) '(lambda quote call/cc list vector match-lambda values)))
						   (> (tree-leaves (car p)) (- leaves (* branches 2)))
						   (or (not (memq head '(or and)))
						       (= i 1))
						   (not (tree-member 'values (car p)))
						   (let ((header (copy form (make-list i)))
							 (trailer (copy form (make-list (- branches i 1)) (+ i 1)))
							 (disclaimer (if (or (any-procedure? head env)
									     (hash-table-ref no-side-effect-functions head))
									 ""
									 (format #f ", assuming ~A is not a macro," head))))
						     ;; begin=(caar p) here is almost entirely as macro arg
						     ;; (apply env-channel (make-env ...) args) -> (let ((_1_ (make-env ...))) (apply env-channel _1_ args))
						     (lint-format "perhaps~A~%~NC~A ->~%~NC~A" caller disclaimer
								  (+ lint-left-margin 4) #\space
								  (lint-pp `(,@header ,(one-call-and-dots (car p)) ,@trailer))
								  (+ lint-left-margin 4) #\space
								  (if (and (memq (caar p) '(let let*))
									   (list? (cadar p))
									   (not (assq head (cadar p)))) ; actually not intersection header+trailer (map car cadr)
								      (let ((last (let ((body (cddar p)))
										    (list-ref body (- (length body) 1)))))
									(if (< (tree-leaves last) 12)
									    (format #f "(~A ... ~A)"
										    (caar p)
										    (lint-pp `(,@header ,last ,@trailer)))
									    (lint-pp `(let ((_1_ ,(one-call-and-dots (car p))))
											(,@header _1_ ,@trailer)))))
								      (lint-pp `(let ((_1_ ,(one-call-and-dots (car p))))
										  (,@header _1_ ,@trailer)))))
						     #t)))))))))
			     
			     (when (pair? form)
			       ;; save any references to vars in their var-history (type checked later)
			       ;;   this can be fooled by macros, as everywhere else
			       (for-each (lambda (arg)
					   (if (symbol? arg)
					       (let ((v (var-member arg env)))
						 (if (and (var? v)
							  (not (memq form (var-history v))))
						     (set! (var-history v) (cons form (var-history v)))))))
					 form)
			       
			       (if (set!? form env)
				   (set-set (cadr form) caller form env)))
			     
			     (if (var? v)
				 (if (and (memq (var-ftype v) '(define lambda define* lambda*))
					  (not (memq caller (var-scope v))))
				     (let ((cv (var-member caller env)))
				       (set! (var-scope v) 
					     (cons (if (and (var? cv)
							    (memq (var-ftype cv) '(define lambda define* lambda*))) ; named-let does not define ftype
						       caller
						       (cons caller env))
						   (var-scope v)))))
				 (begin
				   (cond ((hash-table-ref special-case-functions head)
					  => (lambda (f)
					       (f caller head form env))))
				   
				   ;; change (list ...) to '(....) if it's safe as a constant list
				   ;;   and (vector ...) -> #(...) 
				   (if (and (pair? (cdr form))
					    (hash-table-ref no-side-effect-functions head)
					    (not (memq head unsafe-makers)))
				       (for-each (lambda (p)
						   (if (let constable? ((cp p))
							 (and (pair? cp)
							      (memq (car cp) '(list vector))
							      (pair? (cdr cp))
							      (every? (lambda (inp) 
									(or (code-constant? inp)
									    (constable? inp)))
								      (cdr cp))))
						       (lint-format "perhaps ~A -> ~A~A" caller 
								    (truncated-list->string p)
								    (if (eq? (car p) 'list) "'" "") 
								    (object->string (eval p)))))
						 (cdr form)))
				   
				   (if (and (not (= line-number last-simplify-numeric-line-number))
					    (hash-table-ref numeric-ops head)
					    (proper-tree? form))
				       (let ((val (simplify-numerics form env)))
					 (if (not (equal-ignoring-constants? form val))
					     (begin
					       (set! last-simplify-numeric-line-number line-number)
					       ;; (+ 1 2) -> 3, and many others
					       (lint-format "perhaps ~A" caller (lists->string form val))))))
				   
				   ;; if a var is used before it is defined, the var history and ref/set
				   ;;   info needs to be saved until the definition, so other-identifiers collects it
				   (unless (defined? head (rootlet))
				     (hash-table-set! other-identifiers head 
						      (if (not (hash-table-ref other-identifiers head))
							  (list form)
							  (cons form (hash-table-ref other-identifiers head))))))) 
			     
			     ;; ----------------
			     ;; (f ... (if A B C) (if A D E) ...) -> (f ... (if A (values B D) (values C E)) ...)
			     ;;    these happen up to almost any number of clauses 
			     ;;    need true+false in every case, and need to be contiguous
			     ;;    case/cond happen here, but very rarely in a way we can combine via values
			     
			     (unless (any-macro? head env) ; actually most macros are safe here...
			       (let ((p (member 'if (cdr form) (lambda (x q)
								 (and (pair? q)
								      (eq? (car q) 'if)      ; it's an if expression
								      (pair? (cdr q))
								      (pair? (cddr q))       ; there's a true branch
								      (pair? (cdddr q))))))) ;   and a false branch (similarly below)
				 (when (pair? p)
				   (do ((test (cadar p))
					(q (cdr p) (cdr q)))
				       ((not (and (pair? q)
						  (let ((x (car q)))
						    (and (pair? x)
							 (eq? (car x) 'if)
							 (pair? (cdr x))
							 (equal? (cadr x) test)
							 (pair? (cddr x))
							 (pair? (cdddr x))))))
					(unless (eq? q (cdr p))
					  (let ((header (do ((i 1 (+ i 1))
							     (r (cdr form) (cdr r)))
							    ((eq? r p)
							     (copy form (make-list i)))))
						(middle (do ((r p (cdr r))
							     (trues ())
							     (falses ()))
							    ((eq? r q)
							     `(if ,test 
								  (values ,@(reverse trues)) 
								  (values ,@(reverse falses))))
							  (set! trues (cons (caddar r) trues))
							  (set! falses (cons (car (cdddar r)) falses)))))
					    ;; (+ (if A B C) (if A C D) y) -> (+ (if A (values B C) (values C D)) y)
					    (lint-format "perhaps~A ~A" caller
							 (if (side-effect? test env)
							     (format #f " (ignoring ~S's possible side-effects)" test)
							     "")
							 (lists->string form `(,@header ,middle ,@q))))))))))))
			  ((pair? head)
			   (cond ((not (and (pair? (cdr head))
					    (memq (car head) '(lambda lambda*)))))

				 ((and (identity? head)
				       (pair? (cdr form))) ; identity needs an argument
				  ;; ((lambda (x) x) 32) -> 32
				  (lint-format "perhaps ~A" caller (truncated-lists->string form (cadr form))))
				 
				 ((and (null? (cadr head))
				       (pair? (cddr head)))
				  ;; ((lambda () 32) 0) -> 32
				  (lint-format "perhaps ~A" caller 
					       (truncated-lists->string 
						form 
						(if (and (null? (cdddr head))
							 (not (and (pair? (caddr head))
								   (memq (caaddr head) '(define define* define-constant define-macro define-macro*)))))
						    (caddr head)
						    `(let () ,@(cddr head))))))
				 
				 ((and (pair? (cddr head)) ; ((lambda (...) ...) ...) -> (let ...) -- lambda here is ugly and slow
				       (proper-list? (cddr head))
				       (not (any? (lambda (a) (mv-range a env)) (cdr form))))
				  (call-with-exit
				   (lambda (quit)          ; uncountably many things can go wrong with the lambda form
				     (let ((vars ())
					   (vals ()))
				       (do ((v (cadr head) (cdr v))
					    (a (cdr form) (cdr a)))
					   ((not (and (pair? a)
						      (pair? v)))
					    (if (symbol? v)
						(begin
						  (set! vars (cons v vars))
						  (set! vals (cons `(list ,@a) vals)))
						(do ((v v (cdr v)))
						    ((not (pair? v)))
						  (if (not (pair? v))
						      (quit))
						  (if (pair? (car v)) 
						      (begin
							(if (not (pair? (cdar v)))
							    (quit))
							(set! vars (cons (caar v) vars))
							(set! vals (cons (cadar v) vals)))
						      (begin
							(set! vars (cons (car v) vars))
							(set! vals (cons #f vals)))))))
					 (set! vars (cons ((if (pair? (car v)) caar car) v) vars))
					 (set! vals (cons (car a) vals)))
				       ;; ((lambda* (a b) (+ a b)) 1) -> (let ((a 1) (b #f)) (+ a b))
				       (lint-format "perhaps ~A" caller
						    (lists->string form
								   `(,(if (or (eq? (car head) 'lambda)
									      (not (pair? (cadr head)))
									      (null? (cdadr head)))
									  'let 'let*)
								     ,(map list (reverse vars) (reverse vals))
								     ,@(cddr head))))))))))
		    
			  ((and (procedure? head)
				(memq head '(#_{list} #_{apply_values} #_{append})))
			   (for-each (lambda (p)
				       (let ((sym (and (symbol? p) p)))
					 (when sym
					   (let ((v (var-member sym env)))
					     (if (var? v)
						 (set-ref sym caller form env)
						 (if (not (defined? sym (rootlet)))
						     (hash-table-set! other-identifiers sym
								      (if (not (hash-table-ref other-identifiers sym))
									  (list form)
									  (cons form (hash-table-ref other-identifiers sym))))))))))
				     (cdr form))
			   
			   (when (and (eq? head #_{list})
				      (not (eq? lint-current-form qq-form)))
			     (let ((len (length form)))
			       (set! qq-form lint-current-form) ; only interested in simplest cases here
			       (case len
				 ((2)
				  (if (and (pair? (cadr form))
					   (eq? (caadr form) #_{apply_values})  ; `(,@x) -> (copy x)
					   (not (qq-tree? (cadadr form))))
				      (lint-format "perhaps ~A" caller
						   (lists->string form
								  (un_{list} (if (pair? (cadadr form))
										 (cadadr form)
										 `(copy ,(cadadr form))))))
				      (if (symbol? (cadr form))                
					  (lint-format "perhaps ~A" caller      ; `(,x) -> (list x) 
						       (lists->string form `(list ,(cadr form)))))))
				 ((3)
				  (if (and (pair? (caddr form))
					   (eq? (caaddr form) #_{apply_values})
					   (not (qq-tree? (cadr (caddr form))))
					   (pair? (cadr form))                  ; `(,@x ,@y) -> (append x y)
					   (eq? (caadr form) #_{apply_values})
					   (not (qq-tree? (cadadr form))))
				      (lint-format "perhaps ~A" caller
						   (lists->string form 
								  `(append ,(un_{list} (cadadr form)) 
									   ,(un_{list} (cadr (caddr form))))))))
				 (else 
				  (if (every? (lambda (a)                       ; `(,@x ,@y etc) -> (append x y ...)
						(and (pair? a)
						     (eq? (car a) #_{apply_values})
						     (not (qq-tree? (cdr a)))))
					      (cdr form))
				      (lint-format "perhaps ~A" caller
						   (lists->string form `(append ,@(map (lambda (a)
											 (un_{list} (cadr a)))
										       (cdr form)))))))
				 )))))
		    
		    (let ((vars env))
		      (for-each
		       (lambda (f)
			 (set! vars (lint-walk caller f vars)))
		       form))))
	      env))))))
	
    (define (lint-walk caller form env)
      (cond ((symbol? form)
	     (if (memq form '(+i -i))
		 (format outport "~NC~A is not a number in s7~%" lint-left-margin #\space form))
	     (set-ref form caller #f env)) ; returns env

	    ((pair? form)
	     (lint-walk-pair caller form env))

	    ((string? form)
	     (let ((len (length form)))
	       (if (and (> len 16)
			(string=? form (make-string len (string-ref form 0))))
		   ;; "*****************************" -> (format #f "~NC" 29 #\\*)
		   (lint-format "perhaps ~S -> ~A" caller form `(format #f "~NC" ,len ,(string-ref form 0)))))
	     env)

	    ((vector? form)
	     (let ((happy #t))
	       (for-each
		(lambda (x)
		  (when (and (pair? x)
			     (eq? (car x) 'unquote))
		    (lint-walk caller (cadr x) env) ; register refs
		    (set! happy #f)))
		form)
	       ;; (begin (define x 1) `#(,x))
	       (if (not happy)   ; these are used exactly 4 times (in a test suite!) in 2 million lines of open source scheme code
		   (lint-format "quasiquoted vectors are not supported: ~A" caller form)))
	           ;; `(x #(,x)) for example will not work in s7, but `(,x ,(vector x)) will 
	     env)
	    
	    (else
	     env)))

    
    ;; -------- lint-file --------
    (define *report-input* #t)
    ;; lint-file is called via load etc above and it's a pain to thread this variable all the way down the call chain
    
    (define (lint-file-1 file env)
      (set! linted-files (cons file linted-files))
      (let ((fp (if (input-port? file)
		    file
		    (begin
		      (set! *current-file* file)
		      (catch #t
			(lambda ()
			  (let ((p (open-input-file file)))
			    (if *report-input*
				(format outport 
					(if (and (output-port? outport)
						 (not (memq outport (list *stderr* *stdout*))))
					    (values "~%~NC~%;~A~%" (+ lint-left-margin 16) #\-)
					    ";~A~%")
					file))
			    p))
			(lambda args
			  (format outport "~NCcan't open ~S: ~A~%" lint-left-margin #\space file (apply format #f (cadr args)))
			  #f))))))
	
	(when (input-port? fp)
	  (do ((vars env)
	       (line 0)
	       (last-form #f)
	       (last-line-number -1)
	       (form (read fp) (read fp)))
	      ((eof-object? form) 

	       (if (not (input-port? file))
		   (close-input-port fp))
	       vars)

	    (if (pair? form)
		(set! line (max line (pair-line-number form))))
	    
	    (if (not (or (= last-line-number -1)
			 (side-effect? last-form vars)))
		(format outport "~NCtop-level (line ~D): this has no effect: ~A~%" 
			lint-left-margin #\space last-line-number
			(truncated-list->string last-form)))
	    (set! last-form form)
	    (set! last-line-number line)
	    
	    (if (and (pair? form)
		     (memq (car form) '(define define-macro))
		     (pair? (cdr form))
		     (pair? (cadr form)))
		(let ((f (caadr form)))
		  (if (and (symbol? f)
			   (hash-table-ref built-in-functions f))
		      (format outport "~NCtop-level ~Aredefinition of built-in function ~A: ~A~%" 
			      lint-left-margin #\space 
			      (if (> (pair-line-number form) 0)
				  (format #f "(line ~D) " (pair-line-number form))
				  "")
			      f (truncated-list->string form)))))
	    
	    (set! vars (lint-walk (if (symbol? form) 
				      form 
				      (and (pair? form) 
					   (car form)))
				  form 
				  vars))))))


    (define (lint-file file env)
      ;; (if (string? file) (format *stderr* "lint ~S~%" file))
      
      (if (member file linted-files)
	  env
	  (let ((old-current-file *current-file*)
		(old-pp-left-margin pp-left-margin)
		(old-lint-left-margin lint-left-margin)
		(old-load-path *load-path*))
	    
	    (dynamic-wind
		(lambda ()
		  (set! pp-left-margin (+ pp-left-margin 4))
		  (set! lint-left-margin (+ lint-left-margin 4))
		  (when (and (string? file)
			     (char=? (file 0) #\/))
		    (let ((last-pos 0))
		      (do ((pos (char-position #\/ file (+ last-pos 1)) (char-position #\/ file (+ last-pos 1))))
			  ((not pos)
			   (if (> last-pos 0)
			       (set! *load-path* (cons (substring file 0 last-pos) *load-path*))))
			(set! last-pos pos)))))
		
		(lambda ()
		  (lint-file-1 file env))
		
		(lambda ()
		  (set! pp-left-margin old-pp-left-margin)
		  (set! lint-left-margin old-lint-left-margin)
		  (set! *current-file* old-current-file)
		  (set! *load-path* old-load-path)
		  (if (positive? (length *current-file*))
		      (newline outport)))))))
    
    
    ;;; --------------------------------------------------------------------------------'
    ;;; lint itself
    ;;;
    (let ((documentation "(lint file port) looks for infelicities in file's scheme code")
	  (signature (list #t string? output-port? boolean?)))
      (lambda* (file (outp *output-port*) (report-input #t))
	(set! outport outp)
	(set! other-identifiers (make-hash-table))
	(set! linted-files ())
	(fill! other-names-counts 0)
	(set! last-simplify-boolean-line-number -1)
	(set! last-simplify-numeric-line-number -1)
	(set! last-simplify-cxr-line-number -1)
	(set! last-checker-line-number -1)
	(set! last-cons-line-number -1)
	(set! last-if-line-number -1)
	(set! last-rewritten-internal-define #f)
	(set! line-number -1)
	(set! quote-warnings 0)
	(set! pp-left-margin 0)
	(set! lint-left-margin -3) ; lint-file above adds 4

	(set! big-constants (make-hash-table))
	(set! equable-closures (make-hash-table))

	(set! *report-input* report-input)
	(set! *report-nested-if* (if (integer? *report-nested-if*) (max 3 *report-nested-if*) 4))
	(set! *report-short-branch* (if (integer? *report-short-branch*) (max 0 *report-short-branch*) 12))

	(set! *#readers*
	      (list (cons #\e (lambda (str)
				(if (not (string=? str "e"))
				    (let ((num (string->number (substring str 1))))
				      (if num 
					  (cond ((rational? num)
						 (format outport "~NCthis #e is dumb, #~A -> ~A~%" lint-left-margin #\space str (substring str 1)))
						((not (real? num))
						 (format outport "~NC#e can't handle complex numbers, #~A -> ~A~%" lint-left-margin #\space str num))
						((= num (floor num))
						 (format outport "~NCperhaps #~A -> ~A~%" lint-left-margin #\space str (floor num)))))))
				#f))
		    (cons #\i (lambda (str)
				(if (not (string=? str "i"))
				    (let ((num (string->number (substring str 1))))
				      (if num 
					  (format outport 
						  (if (not (rational? num))
						      (values "~NCthis #i is dumb, #~A -> ~A~%" lint-left-margin #\space str (substring str 1))
						      (values "~NCperhaps #~A -> ~A~%" lint-left-margin #\space str (* 1.0 num)))))))
				#f))
		    (cons #\d (lambda (str)
				(if (and (not (string=? str "d"))
					 (string->number (substring str 1)))
				    (format outport "~NC#d is pointless, #~A -> ~A~%" lint-left-margin #\space str (substring str 1)))
				#f))

		    (cons #\' (lambda (str)                      ; for Guile (and syntax-rules, I think)
				(list 'syntax (if (string=? str "'") (read) (string->symbol str)))))

		    (cons #\` (lambda (str)                      ; for Guile (sigh)
				(list 'quasisyntax (if (string=? str "'") (read) (string->symbol str)))))

		    (cons #\, (lambda (str)                      ; the same, the last is #,@ -> unsyntax-splicing -- right.
				(list 'unsyntax (if (string=? str "'") (read) (string->symbol str)))))

		    (cons #\& (lambda (str)                      ; ancient Guile code
				(make-keyword (substring str 1))))

		    (cons #\\ (lambda (str)
				(cond ((assoc str '(("\\x0"        . #\null)
						    ("\\x7"        . #\alarm)
						    ("\\x8"        . #\backspace)
						    ("\\x9"        . #\tab)
						    ("\\xd"        . #\return)
						    ("\\xa"        . #\newline)
						    ("\\1b"        . #\escape)
						    ("\\x20"       . #\space)
						    ("\\x7f"       . #\delete)))
				       => (lambda (c)
					    (format outport "~NC#\\~A is ~W~%" lint-left-margin #\space (substring str 1) (cdr c)))))
				#f))

		    (cons #\! (lambda (str)
				(if (member str '("!optional" "!default" "!rest" "!key" "!aux" "!false" "!true" "!r6rs") string-ci=?) ; for MIT-scheme
				    (make-keyword (substring str 1))
				    (let ((lc (str 0))) ; s7 should handle this, but...
				      (do ((c (read-char) (read-char)))
					  ((or (and (eof-object? c)
						    (or (format outport "~NCunclosed block comment~%" lint-left-margin #\space)
							#t))
					       (and (char=? lc #\!)
						    (char=? c #\#)))
					   #f)
					(set! lc c))))))))
	
	;; try to get past all the # and \ stuff in other Schemes
	;;   main remaining problem: [] used as parentheses (Gauche and Chicken for example)
	(set! (hook-functions *read-error-hook*)  
	      (list (lambda (h)
		      (let ((data (h 'data))
			    (line (port-line-number)))
			(if (not (h 'type))
			    (begin
			      (format outport "~NCreader[~A]: unknown \\ usage: \\~C~%" lint-left-margin #\space line data)
			      (set! (h 'result) data))
			    (begin
			      (format outport "~NCreader[~A]: unknown # object: #~A~%" lint-left-margin #\space line data)
			      (set! (h 'result)
				    (catch #t
				      (lambda ()
					(case (data 0)
					  ((#\;) (read) (values))
					  
					  ((#\T) (string=? data "T"))
					  ((#\F) (and (string=? data "F") ''#f))

					  ((#\X #\B #\O #\D) 
					   (let ((num (string->number (substring data 1) (case (data 0) ((#\X) 16) ((#\O) 8) ((#\B) 2) ((#\D) 10)))))
					     (if (number? num)
						 (begin
						   (format outport "~NCuse #~A~A not #~A~%" 
							   lint-left-margin #\space 
							   (char-downcase (data 0)) (substring data 1) data)
						   num)
						 (string->symbol data))))
					  
					  ((#\l #\z)
					   (let ((num (string->number (substring data 1)))) ; Bigloo (also has #ex #lx #z and on and on)
					     (if (number? num)
						 (begin
						   (format outport "~NCjust omit this silly #~C!~%" lint-left-margin #\space (data 0))
						   num)
						 (string->symbol data))))
					  
					  ((#\u) ; for Bigloo
					   (if (string=? data "unspecified")
					       (format outport "~NCuse #<unspecified>, not #unspecified~%" lint-left-margin #\space))
					   ;; #<unspecified> seems to hit the no-values check?
					   (string->symbol data))
					  ;; Bigloo also seems to use #" for here-doc concatenation??
					  
					  ((#\v) ; r6rs byte-vectors?
					   (if (string=? data "vu8")
					       (format outport "~NCuse #u8 in s7, not #vu8~%" lint-left-margin #\space))
					   (string->symbol data))
					  
					  ((#\>) ; for Chicken, apparently #>...<# encloses in-place C code
					   (do ((last #\#)
						(c (read-char) (read-char))) 
					       ((and (char=? last #\<) 
						     (char=? c #\#)) 
						(values))
					     (if (char=? c #\newline)
						 (set! (port-line-number ()) (+ (port-line-number) 1)))
					     (set! last c)))
					  
					  ((#\<) ; Chicken also, #<<EOF -> EOF
					   (if (and (char=? (data 1) #\<)
						    (> (length data) 2))
					       (do ((end (substring data 2))
						    (c (read-line) (read-line)))
						   ((string-position end c)
						    (values)))
					       (string->symbol data)))
					  
					  ((#\\) 
					   (cond ((assoc data '(("\\newline"   . #\newline)
								("\\return"    . #\return)
								("\\space"     . #\space)
								("\\tab"       . #\tab)
								("\\null"      . #\null)
								("\\nul"       . #\null)
								("\\linefeed"  . #\linefeed)
								("\\alarm"     . #\alarm)
								("\\esc"       . #\escape)
								("\\escape"    . #\escape)
								("\\rubout"    . #\delete)
								("\\delete"    . #\delete)
								("\\backspace" . #\backspace)
								("\\page"      . #\xc)
								("\\altmode"   . #\escape)
								("\\bel"       . #\alarm) ; #\x07
								("\\sub"       . #\x1a)
								("\\soh"       . #\x01)
								
								;; these are for Guile
								("\\vt"        . #\xb)
								("\\bs"        . #\backspace)
								("\\cr"        . #\newline)
								("\\sp"        . #\space)
								("\\lf"        . #\linefeed)
								("\\nl"        . #\null)
								("\\ht"        . #\tab)
								("\\ff"        . #\xc)
								("\\np"        . #\xc))
							 string-ci=?)
						  => (lambda (c)
						       (format outport "~NCperhaps use ~W instead~%" (+ lint-left-margin 4) #\space (cdr c))
						       (cdr c)))
						 (else 
						  (string->symbol (substring data 1)))))
					  (else 
					   (string->symbol data))))
				      (lambda args #f)))))))))
	
	(let ((vars (lint-file file ())))
	  (set! lint-left-margin (max lint-left-margin 1))

	  (when (pair? vars)
	    (if *report-multiply-defined-top-level-functions*
		(for-each
		 (lambda (var)
		   (let ((var-file (hash-table-ref *top-level-objects* (car var))))
		     (if (not var-file)
			 (hash-table-set! *top-level-objects* (car var) *current-file*)
			 (if (and (string? *current-file*)
				  (not (string=? var-file *current-file*)))
			     (format outport "~NC~S is defined at the top level in ~S and ~S~%" 
				     lint-left-margin #\space 
				     (car var) var-file *current-file*)))))
		 vars))
	    
	    (if (string? file)
		(report-usage top-level: "" vars vars))))

	(for-each 
	 (lambda (p)
	   (if (or (> (cdr p) 5)
		   (and (> (cdr p) 3) 
			(> (length (car p)) 12)))
	       (format outport "~A~A occurs ~D times~%"
		       (if (pair? (car p)) "'" "")
		       (truncated-list->string (car p)) (cdr p))))
	 big-constants)

	(if (and *report-undefined-identifiers*
		 (positive? (hash-table-entries other-identifiers)))
	    (let ((lst (sort! (map car other-identifiers) (lambda (a b)
							    (string<? (symbol->string a) (symbol->string b))))))
	      (format outport "~NCth~A identifier~A not defined~A: ~{~S~^ ~}~%"
		      lint-left-margin #\space 
		      (if (= (hash-table-entries other-identifiers) 1)
			  (values "is" " was")
			  (values "e following" "s were"))
		      (if (string? file) (format #f " in ~S" file) "")
		      lst)
	      (fill! other-identifiers #f)))))))
	      



;;; --------------------------------------------------------------------------------
;;; this reads an HTML file, finds likely-looking scheme code, and runs lint over it.
;;;    called on all snd files in hg.scm

(define (html-lint file)
  
  (define (remove-markups str)
    (let ((tpos (string-position "<b>" str)))
      (if tpos
	  (let ((epos (string-position "</b>" str)))
	    (remove-markups (string-append (substring str 0 tpos)
					   (substring str (+ tpos 3) epos)
					   (substring str (+ epos 4)))))
	  (let ((apos (string-position "<a " str))
		(epos (string-position "<em " str)))
	    (if (not (or apos epos))
		str
		(let* ((pos ((if (and apos epos) min or) apos epos))
		       (bpos (+ (char-position #\> str (+ pos 1)) 1))
		       (epos (string-position (if (and apos (= pos apos)) "</a>" "</em>") str bpos)))
		  (string-append (substring str 0 pos)
				 (substring str bpos epos)
				 (remove-markups (substring str (+ epos (if (and apos (= apos pos)) 4 5)))))))))))
  
  (define (fixup-html str)
    (let ((pos (char-position #\& str)))
      (if (not pos)
	  str
	  (string-append (substring str 0 pos)
			 (let* ((epos (char-position #\; str pos))
				(substr (substring str (+ pos 1) epos)))
			   (string-append (cond ((assoc substr '(("gt"    . ">") 
								 ("lt"    . "<") 
								 ("mdash" . "-") 
								 ("amp"   . "&"))
							string=?) => cdr)
						(else (format () "unknown: ~A~%" substr)))
					  (fixup-html (substring str (+ epos 1)))))))))
  
  (call-with-input-file file
    (lambda (f)
      (do ((line-num 0 (+ line-num 1))
	   (line (read-line f #t) (read-line f #t)))
	  ((eof-object? line))
	
	;; look for <pre , gather everything until </pre>
	;;   decide if it is scheme code (first char is #\()
	;;   if so, clean out html markup stuff, call lint on that
	
	(let ((pos (string-position "<pre" line)))
	  (when pos
	    (let ((code (substring line (+ (char-position #\> line) 1))))
	      (do ((cline (read-line f #t) (read-line f #t))
		   (rline 1 (+ rline 1)))
		  ((string-position "</pre>" cline)
		   (set! line-num (+ line-num rline)))
 		(set! code (string-append code cline)))
	      
	      ;; is first non-whitespace char #\(? ignoring comments
	      (do ((len (length code))
		   (i 0 (+ i 1)))
		  ((>= i len))
		(let ((c (string-ref code i)))
		  (if (not (char-whitespace? c))
		      (if (char=? c #\;)
			  (set! i (char-position #\newline code i))
			  (begin
			    (set! i (+ len 1))
			    (if (char=? c #\()
				(catch #t
				  (lambda ()
				    (let ((outstr (call-with-output-string
						   (lambda (op)
						     (call-with-input-string 
							 (object->string (with-input-from-string 
									     (fixup-html (remove-markups code))
									   read)
									 #t) ; write, not display
						       (lambda (ip)
							 (let-temporarily ((*report-shadowed-variables* #t))
							   (lint ip op #f))))))))
				      (if (> (length outstr) 1)               ; possible newline at end
					  (format () ";~A ~D: ~A~%" file line-num outstr))))
				  (lambda args
				    (format () ";~A ~D, error in read: ~A ~A~%" file line-num args
					    (fixup-html (remove-markups code))))))))))))))))))


;;; --------------------------------------------------------------------------------
;;; and this reads C code looking for s7_eval_c_string. No attempt here to 
;;;   handle weird cases.

(define (C-lint file)
  (call-with-input-file file
    (lambda (f)
      (do ((line-num 0 (+ line-num 1))
	   (line (read-line f #t) (read-line f #t)))
	  ((eof-object? line))

	;; look for s7_eval_c_string, get string arg without backslashes, call lint
	(let ((pos (string-position "s7_eval_c_string(sc, \"(" line)))
	  (when pos
	    (let ((code (substring line (+ pos (length "s7_eval_c_string(sc, \"")))))
	      (if (not (string-position "\");" code))
		  (do ((cline (read-line f #t) (read-line f #t))
		       (rline 1 (+ rline 1)))
		      ((string-position "\");" cline)
		       (set! code (string-append code cline))
		       (set! line-num (+ line-num rline)))
		    (set! code (string-append code cline))))
	      
	      (let ((len (string-position "\");" code)))
		(set! code (substring code 0 len))
		
		;; clean out backslashes
		(do ((i 0 (+ i 1)))
		    ((>= i (- len 3)))
		  (if (char=? (code i) #\\)
		      (cond ((char=? (code (+ i 1)) #\n)
			     (set! (code i) #\space)
			     (set! (code (+ i 1)) #\space))
			    
			    ((memv (code (+ i 1)) '(#\newline #\"))
			     (set! (code i) #\space))
			    
			    ((and (char=? (code (+ i 1)) #\\)
				  (char=? (code (- i 1)) #\#))
			     (set! (code (- i 1)) #\space)
			     (set! (code i) #\#))))))
	      (catch #t
		(lambda ()
		  (let ((outstr (call-with-output-string
				 (lambda (op)
				   (call-with-input-string code
				     (lambda (ip)
				       (let-temporarily ((*report-shadowed-variables* #t))
					 (lint ip op #f))))))))
		    (if (> (length outstr) 1) ; possible newline at end
			(format () ";~A ~D: ~A~%" file line-num outstr))))
		(lambda args
		  (format () ";~A ~D, error in read: ~A ~A~%" file line-num args code))))))))))


;;; --------------------------------------------------------------------------------
#|
;;; external use of lint contents (see also snd-lint.scm):
(for-each (lambda (f) 
            (if (not (hash-table-ref (*lint* 'no-side-effect-functions) (car f)))
                (format *stderr* "~A " (car f))))
          (*lint* 'built-in-functions))

;;; get rid of []'s! (using Snd)
(define (edit file)
  (let* ((str (file->string file))
	 (len (length str)))
    (do ((i 0 (+ i 1)))
	((= i len))
      (case (str i)
	((#\]) (set! (str i) #\)))
	((#\[) (set! (str i) #\())))
    (call-with-output-file file
      (lambda (p)
	(display str p)))
    #f))
|#

;;; --------------------------------------------------------------------------------
;;; TODO:
;;;
;;; code-equal if/when/unless/cond, case: any order of clauses, let: any order of vars, etc, zero/=0
;;;   include named-lets in this search
;;;   these should translate when/unless first -> if?
;;;   (abs|magnitude (- x y)) is reversible internally
;;; indentation is confused in pp by if expr+values?, pp handling of (list ((lambda...)..)) is bad
;;; there are now lots of cases where we need to check for values (/ as invert etc)
;;;   the ((lambda ...)) -> let rewriter is still tricked by values
;;; for scope calc, each macro call needs to be expanded or use out-vars?
;;;   if we know a macro's value, expand via macroexpand each time encountered and run lint on that? [see tmp for expansion]
;;; hg-results has a lot of changes
;;; lint output needs to be organized somehow
;;; define-macro used once -> expand? -- would have to be a local definition [see define-walker]
;;; accessor rep 9349 misses if <expr> sym, and let name ((arg sym)) -- how to store these in the history?
;;;   [<syntax> sym]? -- need to check binding forms etc
;;; do->map similar to do->for-each? named-let|recursive func->for-each|map
;;;   what about do+int-step from 0+1 using list|vector|string-ref?
;;; for-each/map stepping explicitly (not using added args), or similarly for do
;;; named-let->do if one arg is counter, ->for-each if only null? ends
;;;
;;; 148 24013 656883
