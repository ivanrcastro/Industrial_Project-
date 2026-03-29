using System.Text;
using System.Collections.Concurrent; // Para o Dicionário seguro
using MQTTnet;
using MQTTnet.Client;
using Spectre.Console;

var mqttFactory = new MqttFactory();
using var mqttClient = mqttFactory.CreateMqttClient();

// --- A MUDANÇA: Dicionário para guardar múltiplos robôs ---
// Chave: ID do Robô | Valor: (Bateria, Status, Posição)
var fleet = new ConcurrentDictionary<string, (string Bat, string Stat, string Pos)>();

var options = new MqttClientOptionsBuilder()
    .WithTcpServer("127.0.0.1", 1883)
    .Build();

mqttClient.ApplicationMessageReceivedAsync += e => {
    var json = Encoding.UTF8.GetString(e.ApplicationMessage.PayloadSegment);
    
    string id = ExtractJsonValue(json, "id");
    string bat = ExtractJsonValue(json, "battery") + "%";
    string stat = ExtractJsonValue(json, "status");
    string x = ExtractJsonValue(json, "x"); // Extrair X da Bridge
    string y = ExtractJsonValue(json, "y"); // Extrair Y da Bridge

    // Atualiza ou adiciona o robô no dicionário
    fleet[id] = (bat, stat, $"X:{x} Y:{y}");
    return Task.CompletedTask;
};

await mqttClient.ConnectAsync(options);
await mqttClient.SubscribeAsync("factory/fleet/status");

await AnsiConsole.Live(CreateDashboard())
    .StartAsync(async ctx => {
        while (true) {
            ctx.UpdateTarget(CreateDashboard());

            if (Console.KeyAvailable) {
                var key = Console.ReadKey(true).Key;
                string target = "";
                string command = "";

                // MAPEAMENTO SIMPLES (Evita Shift/Alt que falham no Linux)
                switch(key) {
                    case ConsoleKey.D1: target = "AMR_01"; command = "EMERGENCY_STOP"; break;
                    case ConsoleKey.Q:  target = "AMR_01"; command = "RESET"; break;
                    case ConsoleKey.A:  target = "AMR_01"; command = "CHARGE"; break;

                    case ConsoleKey.D2: target = "AMR_02"; command = "EMERGENCY_STOP"; break;
                    case ConsoleKey.W:  target = "AMR_02"; command = "RESET"; break;
                    case ConsoleKey.S:  target = "AMR_02"; command = "CHARGE"; break;

                    case ConsoleKey.X:  target = "ALL";    command = "EMERGENCY_STOP"; break;
                }

                if (!string.IsNullOrEmpty(target)) {
                    string payload = $"{{\"target\":\"{target}\",\"cmd\":\"{command}\"}}";
                    var msg = new MqttApplicationMessageBuilder()
                        .WithTopic("factory/fleet/control")
                        .WithPayload(payload)
                        .Build();
                    await mqttClient.PublishAsync(msg);
                    // O MarkupLine limpa o log para não estragar a tabela
                    AnsiConsole.MarkupLine($"[bold yellow]>>> CMD:[/] [white]{command}[/] -> [blue]{target}[/]");
                }
            }
            await Task.Delay(100); 
        }
    });

// Função que agora itera sobre todos os robôs encontrados
Table CreateDashboard() {
    var table = new Table()
        .Border(TableBorder.Rounded)
        .BorderColor(Color.Yellow)
        .Title("[bold yellow]FACTORY FLEET MANAGER[/]")
        // Forçar a Caption com cores fortes para ser visível
        .Caption("\n[bold red]1,2:[/] STOP | [bold green]Q,W:[/] RESET | [bold yellow]A,S:[/] CHARGE | [bold white]X:[/] STOP ALL");

    table.AddColumn("[blue]Robot ID[/]");
    table.AddColumn("[green]Battery[/]");
    table.AddColumn("[white]Status[/]");
    table.AddColumn("[grey]Position (X,Y)[/]");

    // Importante: Se o dicionário estiver vazio, a tabela não aparece
    if (fleet.IsEmpty) {
        table.AddRow("[red]A aguardar robôs...[/]", "---", "---", "---");
    }

    foreach (var robot in fleet) {
        string bat = robot.Value.Bat;
        string stat = robot.Value.Stat;
        
        // Lógica de cores
        string batColor = "green";
        if (double.TryParse(bat.Replace("%", ""), out double b)) {
            batColor = b < 20 ? "red" : (b < 50 ? "yellow" : "green");
        }
        string statColor = stat == "EMERGENCY_STOP" ? "red bold blink" : "green";

        table.AddRow(
            $"[white]{robot.Key}[/]", 
            $"[{batColor}]{bat}[/]", 
            $"[{statColor}]{stat}[/]",
            $"[grey]{robot.Value.Pos}[/]"
        );
    }
    return table;
}
string ExtractJsonValue(string json, string key) {
    try {
        string searchKey = $"\"{key}\":";
        int start = json.IndexOf(searchKey);
        if (start == -1) return "---";
        
        start += searchKey.Length;

        // Avança enquanto encontrar espaços, aspas ou dois pontos extra
        while (start < json.Length && (json[start] == ' ' || json[start] == '"' || json[start] == ':')) 
            start++;

        // Lê até encontrar o fim do valor
        int end = json.IndexOfAny(new char[] { ',', '}', '"' }, start);
        if (end == -1) end = json.Length;

        string result = json.Substring(start, end - start).Trim();
        return string.IsNullOrEmpty(result) ? "0.0" : result;
    } catch { return "err"; }
}