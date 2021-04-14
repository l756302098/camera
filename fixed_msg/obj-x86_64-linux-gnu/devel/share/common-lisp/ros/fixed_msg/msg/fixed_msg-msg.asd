
(cl:in-package :asdf)

(defsystem "fixed_msg-msg"
  :depends-on (:roslisp-msg-protocol :roslisp-utils )
  :components ((:file "_package")
    (:file "sync_data" :depends-on ("_package_sync_data"))
    (:file "_package_sync_data" :depends-on ("_package"))
  ))