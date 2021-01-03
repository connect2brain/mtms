@echo off
%KAFKA_HOME%\bin\windows\kafka-console-consumer.bat --topic %1 --bootstrap-server 127.0.0.1:9092