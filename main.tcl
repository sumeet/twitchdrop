puts "the script is being run"

proc !hello {args} {
  send_message "you said: |${args}|"
}
