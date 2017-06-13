int _TemperatureSensorValues[] = {};
Postion _GeolocalizationSensorValues[] = {};
int _PressionValues[] = {};

static int _TemperatureSensor_count;
static int _GeolocalizationSensor_count;
static int _PressionSensor_count;

static void *readTemperatureSensor (void *arg)
{
    struct periodic_info info;

    printf ("Thread 1 period 10ms\n");
    make_periodic (10000, &info);
    while (1)
    {
        _TemperatureSensor_count++;
        wait_period (&info);
    }
    return NULL;
}

static void *readGeolocalizationSensor (void *arg)
{
    struct periodic_info info;

    printf ("Thread 2 period 20ms\n");
    make_periodic (20000, &info);
    while (1)
    {
        _GeolocalizationSensor_count++;
        wait_period (&info);
    }
    return NULL;
}

static void *readPressionSensor (void *arg)
{
    struct periodic_info info;

    printf ("Thread 3 period 30ms\n");
    make_periodic (30000, &info);
    while (1)
    {
        _PressionSensor_count++;
        wait_period (&info);
    }
    return NULL;
}