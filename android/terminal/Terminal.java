package studio.murmur.system001;

import android.app.*;
import android.content.*;
import android.hardware.usb.*;
import android.os.*;
import android.graphics.Typeface;
import android.view.View;
import android.widget.*;
import java.io.*;
import java.nio.charset.StandardCharsets;
import java.util.*;
import java.util.concurrent.ArrayBlockingQueue;
import java.util.regex.*;

public final class Terminal extends Activity {
    private static final String PERMIT = "studio.murmur.system001.USB";
    private static final Pattern ID = Pattern.compile("^# BLACKNODE \\S+ profile=([1-4]) boot=[0-9a-fA-F]{8} job=\\S+ heap=\\d+ minheap=\\d+ uptime_ms=\\d+ reset=\\d+$");
    private final ArrayBlockingQueue<String> writes = new ArrayBlockingQueue<>(16);
    private UsbManager usb;
    private UsbDevice selected;
    private TextView state, output;
    private Spinner commands;
    private Button send, connect;
    private int session;
    private volatile int live;
    private final StringBuilder transcript = new StringBuilder();
    private final BroadcastReceiver receiver = new BroadcastReceiver() {
        @Override public void onReceive(Context c, Intent intent) {
            UsbDevice device = intent.getParcelableExtra(UsbManager.EXTRA_DEVICE);
            if (selected == null || !selected.equals(device)) return;
            if (UsbManager.ACTION_USB_DEVICE_DETACHED.equals(intent.getAction())) end("USB disconnected");
            else if (PERMIT.equals(intent.getAction())) {
                if (usb.hasPermission(device)) startDevice(device);
                else end("USB permission denied");
            }
        }
    };
    @Override public void onCreate(Bundle saved) {
        super.onCreate(saved);
        usb = (UsbManager)getSystemService(USB_SERVICE);
        LinearLayout layout = new LinearLayout(this);
        layout.setOrientation(LinearLayout.VERTICAL);
        layout.setPadding(24, 48, 24, 24);
        state = new TextView(this);
        state.setText("BLACKNODE C3 · USB disconnected");
        layout.addView(state);
        connect = new Button(this);
        connect.setText("Select USB device");
        connect.setOnClickListener(v -> choose());
        layout.addView(connect);
        commands = new Spinner(this);
        layout.addView(commands);
        send = new Button(this);
        send.setText("Send command");
        send.setEnabled(false);
        send.setOnClickListener(v -> {
            if (!writes.offer((String)commands.getSelectedItem())) state.setText("Command queue full");
        });
        layout.addView(send);
        output = new TextView(this);
        output.setTypeface(Typeface.MONOSPACE);
        output.setTextIsSelectable(true);
        ScrollView scroll = new ScrollView(this);
        scroll.addView(output);
        layout.addView(scroll, new LinearLayout.LayoutParams(-1, 0, 1));
        Button save = new Button(this);
        save.setText("Save transcript");
        save.setOnClickListener(v -> startActivityForResult(new Intent(Intent.ACTION_CREATE_DOCUMENT)
            .setType("text/plain").addCategory(Intent.CATEGORY_OPENABLE)
            .putExtra(Intent.EXTRA_TITLE, "murmur-terminal.txt"), 7));
        layout.addView(save);
        setContentView(layout);
        IntentFilter filter = new IntentFilter(PERMIT);
        filter.addAction(UsbManager.ACTION_USB_DEVICE_DETACHED);
        if (Build.VERSION.SDK_INT >= 33) registerReceiver(receiver, filter, RECEIVER_NOT_EXPORTED);
        else registerReceiver(receiver, filter);
    }
    @Override protected void onActivityResult(int request, int result, Intent data) {
        super.onActivityResult(request, result, data);
        if (request != 7 || result != RESULT_OK || data == null || data.getData() == null) return;
        try (OutputStream file = getContentResolver().openOutputStream(data.getData(), "wt")) {
            file.write(transcript.toString().getBytes(StandardCharsets.UTF_8));
        } catch (Exception e) { state.setText("Save failed · " + e.getMessage()); }
    }
    private void choose() {
        List<UsbDevice> devices = new ArrayList<>();
        for (UsbDevice d : usb.getDeviceList().values()) {
            if (d.getVendorId() == 0x303a && d.getProductId() == 0x1001) devices.add(d);
        }
        if (devices.isEmpty()) { end("No Espressif USB Serial/JTAG device detected"); return; }
        String[] labels = new String[devices.size()];
        for (int i=0; i<labels.length; i++) labels[i] = devices.get(i).getDeviceName();
        new AlertDialog.Builder(this).setTitle("Select BLACKNODE USB port").setItems(labels, (d,i) -> {
            end("Waiting for USB permission");
            selected = devices.get(i);
            if (usb.hasPermission(selected)) startDevice(selected);
            else usb.requestPermission(selected, PendingIntent.getBroadcast(this, session,
                new Intent(PERMIT).setPackage(getPackageName()), PendingIntent.FLAG_MUTABLE));
        }).show();
    }
    private void end(String message) {
        live = ++session;
        selected = null;
        writes.clear();
        send.setEnabled(false);
        state.setText(message);
    }
    private void startDevice(UsbDevice device) {
        int token = ++session;
        live = token;
        state.setText("Reading firmware identity");
        new Thread(() -> communicate(device, token), "murmur-usb").start();
    }
    private void communicate(UsbDevice device, int token) {
        UsbDeviceConnection link = usb.openDevice(device);
        try {
            if (link == null) throw new IOException("Cannot open port");
            UsbEndpoint input = null, outputPipe = null;
            for (int i=0; i<device.getInterfaceCount(); i++) {
                UsbInterface face = device.getInterface(i);
                if (face.getInterfaceClass() == UsbConstants.USB_CLASS_COMM) {
                    if (!link.claimInterface(face, true)) throw new IOException("Control interface busy");
                    if (link.controlTransfer(0x21, 0x22, 3, face.getId(), null, 0, 500) < 0)
                        throw new IOException("USB line control failed");
                }
                if (face.getInterfaceClass() != UsbConstants.USB_CLASS_CDC_DATA) continue;
                if (!link.claimInterface(face, true)) throw new IOException("Data interface busy");
                for (int e=0; e<face.getEndpointCount(); e++) {
                    UsbEndpoint pipe = face.getEndpoint(e);
                    if (pipe.getType() != UsbConstants.USB_ENDPOINT_XFER_BULK) continue;
                    if (pipe.getDirection() == UsbConstants.USB_DIR_IN) input = pipe;
                    else outputPipe = pipe;
                }
            }
            if (input == null || outputPipe == null) throw new IOException("CDC bulk endpoints missing");
            byte[] buffer = new byte[512];
            ByteArrayOutputStream line = new ByteArrayOutputStream();
            long started = SystemClock.elapsedRealtime(), probe = 0;
            boolean identified = false;
            while (live == token) {
                long now = SystemClock.elapsedRealtime();
                if (!identified && now-started > 7500) throw new IOException("BLACKNODE identity timed out");
                String cmd = null;
                if (!identified && now >= probe) { cmd = "status"; probe = now+1000; }
                else if (identified) cmd = writes.poll();
                if (cmd != null) {
                    byte[] frame = (cmd+"\n").getBytes(StandardCharsets.US_ASCII);
                    if (link.bulkTransfer(outputPipe, frame, frame.length, 500) != frame.length)
                        throw new IOException("USB write failed");
                }
                int n = link.bulkTransfer(input, buffer, buffer.length, 100);
                if (n < 0 && !usb.getDeviceList().containsKey(device.getDeviceName()))
                    throw new IOException("USB disconnected");
                for (int j=0; j<n; j++) {
                    if (buffer[j] == '\r') continue;
                    if (buffer[j] != '\n') {
                        if (line.size() >= 4096) throw new IOException("Oversized serial line");
                        line.write(buffer[j]); continue;
                    }
                    String record = new String(line.toByteArray(), StandardCharsets.UTF_8);
                    line.reset();
                    Matcher match = ID.matcher(record);
                    int profile = !identified && match.matches() ? Integer.parseInt(match.group(1)) : 0;
                    if (profile > 0) identified = true;
                    runOnUiThread(() -> { if (live == token) display(record, profile); });
                }
            }
        } catch (Exception e) {
            runOnUiThread(() -> { if (live == token) end(e.getMessage()); });
        } finally { if (link != null) link.close(); }
    }
    private void display(String record, int profile) {
        if (profile > 0) {
            List<String> options = new ArrayList<>(Arrays.asList("status","wifi","ble","stop","export wifi","export ble","export events"));
            if (profile > 1) options.addAll(Arrays.asList("scope","rx","export packets"));
            commands.setAdapter(new ArrayAdapter<>(this, android.R.layout.simple_spinner_dropdown_item, options));
            send.setEnabled(true);
            state.setText("BLACKNODE C3 · profile " + profile);
        }
        transcript.append(record).append('\n');
        if (transcript.length() > 131072) transcript.delete(0, transcript.indexOf("\n", 8192)+1);
        output.setText(transcript.substring(Math.max(0, transcript.length()-8192)));
    }
    @Override protected void onDestroy() {
        end("Closed");
        unregisterReceiver(receiver);
        super.onDestroy();
    }
}
