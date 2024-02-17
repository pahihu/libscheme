(define (macro-expand L)
   (cond
      ((null? L) L)
      ; ((symbol? L) L)
      ; ((number? L) L)
      ; ((string? L) L)
      ; ((boolean? L) L)
      ; ((character? L) L)
      ; ((vector? L) L)
      ((pair? L)
         (let ((rator (car L)))
            (if (and (symbol? rator) (bound? rator) (macro? (eval rator)))
               (apply (eval rator) (cdr L))
               (cons (macro-expand rator) (macro-expand (cdr L))))))
      (#t L)))

(define *scheme-define* define)
(defmacro define X
   ; (display "define: ") (display X) (newline)
   (let ((ret #f))
   (if (and (= 2 (length X)) (pair? (car X)))
      (set! ret (cons '*scheme-define* (cons (car X) (macro-expand (cdr X)))))
      (set! ret (cons '*scheme-define* (cons (car X) (macro-expand (cdr X))))))
      (display ret) (newline)
   ret))

;;; vim:set ts=3 sw=3 et:
