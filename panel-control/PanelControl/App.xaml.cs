using Microsoft.UI.Xaml;

namespace PanelControl;

public partial class App : Application
{
    private Window? _ventanaPrincipal;

    public App()
    {
        InitializeComponent();
    }

    protected override void OnLaunched(LaunchActivatedEventArgs args)
    {
        _ventanaPrincipal = new MainWindow();
        _ventanaPrincipal.Activate();
    }
}
