## Perguntas:

1. Utilize o Systick  para medir o tempo de cada thread e faça um diagrama temporal da execução do código fornecido para diferentes casos de prioridade e tempos (`k_sleep()`) e explique cada caso.
    
    Dica: utilize Frequência de Clock e distribuição para calcular o tempo, se preciso. 
    
2. O que acontece se removermos o `k_sleep()` de uma das threads, explique? Se as threads têm prioridades iguais, qual será executada primeiro? Como funciona threads cooperativas (prioridade -1), explique? 

3. É possível que uma thread nunca seja executada? Em qual combinação de prioridade/tempo isso acontece? Como é conhecido isso? 

4. O que acontece com um a interrupção de hardware (ISR)? Implemente um botão (Botão com Interrupção) com interrupção para mostrar o comportamento e prioridade da ISR.

5. Substitua os comandos de printk por comandos com Logging. Inclua mais informações (como os tempos intermediários do timer) que podem ser utilizados para depuração do código. Discuta a diferença entre usar o Logging, printk, printf e debug.