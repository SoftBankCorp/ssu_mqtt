# Generated by Django 4.2.16 on 2024-09-15 07:56

from django.db import migrations


class Migration(migrations.Migration):

    dependencies = [
        ('myapp', '0004_averagesensordata_rawsensordata'),
    ]

    operations = [
        migrations.DeleteModel(
            name='SensorData',
        ),
        migrations.RenameField(
            model_name='averagesensordata',
            old_name='current_avg',
            new_name='current',
        ),
        migrations.RenameField(
            model_name='averagesensordata',
            old_name='humidity_avg',
            new_name='humidity',
        ),
        migrations.RenameField(
            model_name='averagesensordata',
            old_name='temperature_avg',
            new_name='temperature',
        ),
        migrations.RenameField(
            model_name='averagesensordata',
            old_name='voltage_avg',
            new_name='voltage',
        ),
    ]
