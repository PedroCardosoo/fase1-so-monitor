O que faremos?

teremos dois arquivos que juntos serao responsaveis por informar a um usuario a utilizacao dos recursos no servidor.

    - Um arquivo sera o servidor em si, isto eh, ele sera responsavel por receber do cliente textos vindos por meio de socket, e executara comandos no terminal para saber informacoes da maquina e enviar de volta como texto para o cliente.

    - O outro arquivo sera o cliente propriamente dito, ele ira se conectar a porta do backend por websocket e enviar textos provavelmente ja fixos para rastrear os recursos do servidor.


 Para criar esta conexao por meio de sockets em C somos obrigados a usar funcoes pre-escritas pois estas conseguem conversar com o kernel.
Sao as funcoes ja em ordem de uso:

    SERVIDOR
        socket()
        bind()
        listen()
        accept()    
        recv()
        send() 
        close()


    CLIENTE
        socket()
        connect()
        recv()
        send()
        close()

As funcoes ja sao um pouco auto-explicadas por seus nomes, porem de forma geral temos de criar um socket em ambos os lados. Este 'criar socket' nada mais eh que colocar na memoria do Kernel um bloco de informacoes necessarias para a conexao, como o ip de origem e destino, os ponteiros para os protocolos que serao usados (tcp, udp, ipv4, etc....).
Apos a cricao, de um lado apenas conectamos ao servidor, do outro, no servidor, temos de configurar a porta e informacoes pra comunicacao.

Send e Recovery serven para enviar e ler as mensagens respectivamente.
