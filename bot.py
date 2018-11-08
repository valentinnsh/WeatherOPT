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


helping_msg = "Hellow. I am HutWeatherBot and I can tell you about weather in you hut.\n Just press /start, my fellow beaver friend"
stop_msg = "Ok, I'll stop"
starting_msg = "Let's go, here your weather:"
flag_stop = 1
answering_string = "Here we go. Current temperature ="

@bot.message_handler(commands=['help'])
def send_help_msg(message):
    bot.send_message(message.chat.id, helping_msg)
    
@bot.message_handler(commands=['start'])
def starting_monitoring(message):
    bot.send_message(message.chat.id, starting_msg)
    while(True):
        measure_proc = subprocess.Popen([r"/home/valentine/bosh_tph"], stdout=subprocess.PIPE)
        (answering_string, _) = measure_proc.communicate()
        bot.send_message(message.chat.id, answering_string)
        time.sleep(60)        
        
@bot.message_handler(commands=['stop'])
def stop_monitoring(message):
    bot.send_message(message.chat.id, stop_msg)
    flag_stop = 0

    
#exit(0)

if __name__ == '__main__':
     bot.polling(none_stop=True)
