proc !help {} {
    list "Bot commands: !project !discord !pair" \
         "Let me know if you have any coding questions, or just want me to look at something on-stream."
}

proc !pair {} {
    return "Let's pair together on-stream! I can work together over the following platforms: Discord, VS Code Live Share, JetBrains Code With Me, Replit"
}

proc !project {} {
    return "Twitch bot in Pure C https://github.com/sumeet/twitchdrop"
}
proc !today {} { !project }

proc !discord {} {
    return "Join the Sumcademy Discord server at https://discord.gg/5hHqbk6eRu"
}
