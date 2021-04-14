
(cl:in-package :asdf)

(defsystem "fixed_msg-srv"
  :depends-on (:roslisp-msg-protocol :roslisp-utils )
  :components ((:file "_package")
    (:file "cp_control" :depends-on ("_package_cp_control"))
    (:file "_package_cp_control" :depends-on ("_package"))
  ))