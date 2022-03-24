;;;
;;; List functions
;;;
;;; (foreach Fun L) => #t
;;; (filter Pred L) => L
;;; (partition Pred L) => (Satisfy . NotSatisfy)
;;; (sort Pred L) => L
;;;
(define (foreach Fun L)
   (cond
      ((null? L) #t)
      (#t (Fun (car L))
          (foreach Fun (cdr L)) ) ) )

(define (filter Pred L)
   (if (null? L)
      '()
      (let ((E (car L)))
         (if (Pred E)
            (cons E (filter Pred (cdr L)))
            (filter Pred (cdr L)) ) ) ) )

(define (partition Pred L)
   (if (null? L)
      (cons '() '())
      (let* ((P      (partition Pred (cdr L)))
             (Sat    (car P))
             (NotSat (cdr P))
             (E      (car L)) )
         (if (Pred E)
            (cons (cons E Sat) NotSat)
            (cons         Sat  (cons E NotSat)) ) ) ) )

(define (sort Pred L)
   (cond
      ((< (length L) 2)
         L)
      (#t
         (let* ((Pivot (car L))
                (P (partition
                           (lambda (x) (Pred x Pivot))
                           (cdr L) ) ) )
            (append
               (sort Pred (car P))
               (list Pivot)
               (sort Pred (cdr P)) ) ) ) ) )
