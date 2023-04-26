#include <cpsumon.h>
#include <cpsumoncli.h>
#include <time.h>
#include <string.h>

int print_stats(const char *name, float voltage, float current, float power) {
    return printf(
        "\"%s\":{"
        "\"voltage\": %01.8g, "
        "\"current\": %01.8g, "
        "\"power\": %01.8g",
        name,
        voltage,
        current,
        power
    );
}

int print_stats_12v( const char *name, float voltage, float current, float power, int ocp_enabled, float ocp_limit) {
    return print_stats(name, voltage, current, power) && printf(
        ", \"ocp\": %d,"
        "\"ocp_limit\": %01.8g"
        "}, ",
        ocp_enabled,
        ocp_limit
    );
}

int main (int argc, char * argv[]) {
    int fd;
    struct termios tio;
    int i;

    _psu_type = TYPE_AX760;

    //printf("Corsair AXi Series PSU Monitor\n");
    //printf("(c) 2014 Andras Kovacs - andras@sth.sze.hu\n");
    //printf("-------------------------------------------\n\n");

    const int BUFFSIZE = 26;
    struct tm now;
    char timestr[BUFFSIZE]; // Just enough for ISO8601
    time_t ts = time(NULL);
    gmtime_r(&ts, &now);
    strftime(&timestr, BUFFSIZE, "%FT%T%z", &now);

    printf("{\"timestamp\": \"%s\", ", timestr);

    if (argc < 2) {
        printf("usage: %s <serial port device>\n", argv[0]);
        return 0;
    }

    fd = open_usb(argv[1]);
    if(fd == -1) {
        return -1;
    }
    memset(&tio, 0, sizeof(tio));
    tio.c_cflag = B115200 | CS8 | CLOCAL | CREAD;
    tio.c_iflag = IGNPAR;
    tio.c_oflag = 0;
    tio.c_lflag = 0;
    tio.c_cc[VTIME]= 0;
    tio.c_cc[VMIN] = 1;
    tcsetattr(fd, TCSANOW, &tio);
    tcflush(fd, TCIOFLUSH);

    float f;

    if (setup_dongle(fd) == -1)
        exit(-1);

    if (read_psu_fan_mode(fd, &i) == -1)
        exit(-1);

    if (i == FANMODE_AUTO) {
        printf("\"fan_mode\": \"Auto\", ");
    } else if (i == FANMODE_FIXED) {
        printf("\"fan_mode\": \"Fixed\", ");

        if (read_psu_fan_fixed_percent(fd, &i) == -1)
            exit(-1);
        printf("\"fan_setting\": %d, ", i);
    }

    if (read_psu_fan_speed(fd, &f) == -1)
        exit(-1);
    printf("\"fan_speed\": %01.8g, ", f);
    if (read_psu_temp(fd, &f) == -1)
        exit(-1);
    printf("\"temperature\": %01.8g, ", f);

    if (read_psu_main_power(fd) == -1)
        exit(-1);

    printf("\"voltage\": %01.8g, ", _psumain.voltage);
    printf("\"current\": %01.8g, ", _psumain.current);
    printf("\"power_in\": %01.8g, ", _psumain.inputpower);
    printf("\"power_out\": %01.8g, ", _psumain.outputpower);
    //if (_psu_type == TYPE_AX1500)
    //    printf("Cable type: %s\n", (_psumain.cabletype ? "20 A" : "15 A"));
    printf("\"efficiency\": %01.8g, ", _psumain.efficiency);

    if (read_psu_rail12v(fd) == -1)
        exit(-1);

    int chnnum = (_psu_type == TYPE_AX1500 ? 10 : ((_psu_type == TYPE_AX1200) ? 8 : 6));
    for (i = 0; i < chnnum; i++) {
        printf("\"pcie_rail_%02d\": {"
            "\"voltage\": %01.8g,"
            "\"current\": %01.8g,"
            "\"power\": %01.8g,"
            "\"ocp\": %d,"
            "\"ocp_limit\": %01.8g}, ",
            i,
            _rail12v.pcie[i].voltage,
            _rail12v.pcie[i].current,
            _rail12v.pcie[i].power,
            _rail12v.pcie[i].ocp_enabled,
            _rail12v.pcie[i].ocp_limit
        );
    }

    printf("\"12v_atx_rail\": {"
        "\"voltage\": %01.8g,"
        "\"current\": %01.8g,"
        "\"power\": %01.8g,"
        "\"ocp\": %d,"
        "\"ocp_limit\": %01.8g}, ",
        _rail12v.atx.voltage,
        _rail12v.atx.current,
        _rail12v.atx.power,
        _rail12v.atx.ocp_enabled,
        _rail12v.atx.ocp_limit
    );

    //printf("\"12v_peripheral_rail\":"
    //    "%01.8g,"
    //    "%01.8g,"
    //    "%01.8g,"
    //    "\"ocp\": %d,"
    //    "\"ocp_limit\": %01.8g}, ",
    //    _rail12v.peripheral.voltage,
    //    _rail12v.peripheral.current,
    //    _rail12v.peripheral.power,
    //    _rail12v.peripheral.ocp_enabled,
    //    _rail12v.peripheral.ocp_limit
    //);

    print_stats_12v("12v_peripheral_rail",
        _rail12v.peripheral.voltage,
        _rail12v.peripheral.current,
        _rail12v.peripheral.power,
        _rail12v.peripheral.ocp_enabled,
        _rail12v.peripheral.ocp_limit
    );

    if(read_psu_railmisc(fd) == -1) exit(-1);

    print_stats("5v_rail", _railmisc.rail_5v.voltage, _railmisc.rail_5v.current, _railmisc.rail_5v.power);
    printf("}, ");
    //printf("\"5v_rail\":"
    //    "%01.8g,"
    //    "%01.8g,"
    //    "%01.8g,\n",
    //    _railmisc.rail_5v.voltage,
    //    _railmisc.rail_5v.current,
    //    _railmisc.rail_5v.power);

    // Last one so be sure NOT to include the trailing comma (b/c JSON is stupid)
    //printf("\"3v3_rail\":"
    //    "\"voltage\": %01.8g,"
    //    "%01.8g,"
    //    "%01.8g\n",
    //    _railmisc.rail_3_3v.voltage,
    //    _railmisc.rail_3_3v.current,
    //    _railmisc.rail_3_3v.power
    //);
    print_stats("3v3_rail", _railmisc.rail_3_3v.voltage, _railmisc.rail_3_3v.current, _railmisc.rail_3_3v.power);

    printf("}}\n");

    close(fd);
    return 0;
}

// vi: sw=4
