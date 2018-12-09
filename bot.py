#
# Copyright (c)
# 2018 V.Shishkin
#

# -*- coding: utf-8 -*-

import config
import telebot
import sys
import subprocess
import time

bot = telebot.TeleBot(config.token)


helping_msg = "Hellow. I am HutWeatherBot and I can tell you about weather in you hut.\n Just press /give, when you temperature, pressure and humidity.\nSend /temp if you want to know just temperature\nSend /pres if you want to know just pressure\nSend /hum just for humidity\n\n Enjoy!"
stop_msg = "Ok, I'll stop"
starting_msg = "Let's go, here your weather:"
send_temp_msg = "Current temp is: "
send_pres_msg = "Current pressure is: "
send_hum_msg = "Current humidity is: "

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
    bot.send_message(message.chat.id, send_temp_msg)
    answering_string = subprocess.check_output([r"/home/valentine/bosh_tph"])
    pos = int(answering_string.find("\n".encode('utf8'), 0))
    bot.send_message(message.chat.id, answering_string[0:pos])

@bot.message_handler(commands=['pres'])
def send_pres(message):
    bot.send_message(message.chat.id, send_pres_msg)
    answering_string = subprocess.check_output([r"/home/valentine/bosh_tph"])
    pos1 = int(answering_string.find("\n".encode('utf8'), 0))
    pos2 = int(answering_string.find("\n".encode('utf8'), pos1+1))
    bot.send_message(message.chat.id, answering_string[pos1:pos2])

@bot.message_handler(commands=['hum'])
def send_hum(message):
    bot.send_message(message.chat.id, send_hum_msg)
    answering_string = subprocess.check_output([r"/home/valentine/bosh_tph"])
    pos1 = int(answering_string.find("\n".encode('utf8'), 0))
    pos2 = int(answering_string.find("\n".encode('utf8'), pos1+1))
    pos3 = int(answering_string.find("\n".encode('utf8'), pos2+1))
    bot.send_message(message.chat.id, answering_string[pos2:pos3])
#exit(0)

if __name__ == '__main__':
     bot.polling(none_stop=True)
