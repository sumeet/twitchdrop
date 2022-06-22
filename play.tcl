#!/usr/bin/tclsh

proc tworandoms {} {
    set r1 [expr round(rand()*10)]
    set r2 [expr round(rand()*10)]

    list "Bot commands: !project !discord !pair" \
         "Let me know if you have any coding questions, or just want me to look at something on-stream."
    #list [list more stuff in the first list $r1] [list the second list more stuff $r2]
    #list $r1 $r2
    #return "$r1 $r2"
}

# destructuring set
proc dset {varNames values} {
  foreach varName $varNames value $values {
    upvar 1 $varName var
    set var $value
  }
}


dset {x y} [tworandoms]
puts $x
puts $y
puts "----"
puts $x
puts $y
