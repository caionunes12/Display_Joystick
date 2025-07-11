>>> Link para o vídeo explicativo: https://youtu.be/aTHf2rnl27M?si=kLNkP57DhOwIVUmy

# Display_Joystick
Este algoritmo lê continuamente a posição do potênciometro ( Joystick ) usando a conversão AD do Pico, em Seguida um filtro de média móvel suaviza a leitura, na qual a posição do joystick varia ao longo de toda a resolução do microcontrolador onde a posição é convertida é níveis de tensão (0 a 3,3V) e exibe-se o sinal num display OLED através do protocolo de comunicação I2C. 

Extra : Um gráfico de dispersão ( tensão em função do tempo) também foi implementado para ilustrar a variação do sinal conforme varia-se a posição do Joystick.
