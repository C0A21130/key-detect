using System.Diagnostics;
using System.Text.Json;

namespace MauiApp1;

public class KeyStatus
{
	public string status { get; set; }
}

public partial class MainPage : ContentPage
{
	HttpClient client = new HttpClient();
	JsonSerializerOptions options;

    int count = 0;
    public KeyStatus KeyStatus { get; private set; }

	public MainPage()
	{
		InitializeComponent();
	}

	private void OnCounterClicked(object sender, EventArgs e)
	{
		count++;

		if (count == 1)
			CounterBtn.Text = $"Clicked {count} time";
		else
			CounterBtn.Text = $"Clicked {count} times";

		SemanticScreenReader.Announce(CounterBtn.Text);
	}

    private async void KeyStatusClicked(object sender, EventArgs e)
    {
		var status = await GetStatus();
		KeyStatusBtn.Text = status.status;
		Debug.WriteLine(status);
        SemanticScreenReader.Announce(KeyStatusBtn.Text);
    }

    public async Task<KeyStatus> GetStatus() 
	{
		KeyStatus = new KeyStatus();
		try
		{
			HttpResponseMessage response = await client.GetAsync("http://192.168.2.128:80");
			string json = await response.Content.ReadAsStringAsync();
			KeyStatus = JsonSerializer.Deserialize<KeyStatus>(json, options);
		}
		catch (Exception ex)
		{
			Debug.WriteLine(ex.Message);
		}

		return KeyStatus;
		
	}
}

