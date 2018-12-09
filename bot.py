#
# Copyright (c) 2018 V.Shishkin
#

# -*- coding: utf-8 -*-

import config
import telebot
import sys
import subprocess
import time

bot = telebot.TeleBot(config.token)


helping_msg = "Hellow. I am HutWeatherBot and I can tell you about weather in you hut.\n Just press /give, when you want to know"
stop_msg = "Ok, I'll stop"
starting_msg = "Let's go, here your weather:"
send_temp_msg = "Current temp is: "

@bot.message_handler(commands=['help'])
def send_help_msg(message):
    bot.send_message(message.chat.id, helping_msg)
    
@bot.message_handler(commands=['give'])
def send_weather(message):
    bot.send_message(message.chat.id, starting_msg)
    answering_string = subprocess.check_output([r"/home/valentine/bosh_tph"])
    bot.send_message(message.chat.id, answering_string)
        
@bot.message_handler(commands=['temp'])
def send_temp(message):
    bot.send_message(message.chat.id, starting_msg)
    answering_string = subprocess.check_output([r"/home/valentine/bosh_tph"])
    bot.send_message(message.chat.id, answering_string[0::answering_string.find("\n")])
    
#exit(0)

if __name__ == '__main__':
     bot.polling(none_stop=True)
