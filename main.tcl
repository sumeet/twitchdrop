puts "the script is being run"

# TODO: figure out why we can only pass in varargs (via `args` instead of the number of arguments we want)
proc !hello {args} {
  send_message "you said: |${args}|"
}
