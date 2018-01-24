/*
 * This file is Public Domain and provided only for documentation purposes.
 *
 * Note this is buttugly, please don't actually use this as a UI template.
 *
 * Compile: valac list.vala --pkg gtk+-3.0 --pkg ldm-0.1
 * Run    : ./list
 */
public class MyLister : Gtk.Window
{
    Ldm.Manager manager;
    Gtk.ListBox listing;

    public MyLister()
    {
        // Construct a new manager without hotplug capabilites
        manager = new Ldm.Manager(Ldm.ManagerFlags.NO_MONITOR);

        // Set up main window attributes
        var header = new Gtk.HeaderBar();
        header.set_show_close_button(true);
        set_titlebar(header);
        set_title("List devices");
        set_size_request(550, 400);
        get_settings().gtk_application_prefer_dark_theme =  true;

        // Init UI area
        listing = new Gtk.ListBox();
        var scroll = new Gtk.ScrolledWindow(null, null);
        add(scroll);
        scroll.add(listing);

        // Whack all devices in as simple labels (ugly)
        manager.get_devices().foreach((device)=> {
            var label = new Gtk.Label(@"$(device.vendor) - $(device.name)");
            listing.add(label);
            label.margin = 10;
            label.halign = Gtk.Align.START;

            if (device.has_type(Ldm.DeviceType.GPU)) {
                message("Found a GPU: %s", device.name);
            }
        });

        destroy.connect(Gtk.main_quit);
        show_all();
    }
}

int main(string[] args)
{
    Gtk.init(ref args);
    var lister = new MyLister();
    Gtk.main();
    return 0;
}
