proc !commands {} {
    return "!project !discord !help !pair"
}

proc !help {} {
    return "If you think I might be able to help you code something, just ask anytime. Don't worry about interrupting. If you wanted a list of commands, you should type !commands"
}
proc !pair {} { return "Let's pair on something together on-stream! Either VSCode Live Share or Jetbrains, we'll figure out the logistics." }
proc !project {} {
    return "Learning C writing a Twitch bot -- https://github.com/sumeet/twitchdrop"
}
proc !today {} { !project }
proc !discord {} {
    return "Join the Sumcademy Discord server at https://discord.gg/5hHqbk6eRu"
}