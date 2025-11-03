import asterix
import unittest


class AsterixParseTest(unittest.TestCase):
    def test_ParseCAT048(self):
        sample_filename = asterix.get_sample_file('cat048.raw')
        with open(sample_filename, "rb") as f:
            data = f.read()
            packet = asterix.parse(data)
            self.maxDiff = None
            self.assertIsNotNone(packet)
            self.assertIsNotNone(packet[0])
            self.assertIs(len(packet), 1)
            self.assertTrue('I220' in packet[0])
            self.assertEqual(packet[0]['category'], 48)
            self.assertEqual(packet[0]['len'], 45)
            self.assertEqual(packet[0]['crc'], 'AB659C3E')
            self.assertTrue('ts' in packet[0])
            self.assertEqual(packet[0]['I220']['AA']['val'], 3958284)  # Aircraft Address as integer (0x3C660C = 3958284)
            self.assertEqual(packet[0]['I220']['AA']['desc'], 'Aircraft Address')
            self.assertEqual(packet[0]['I010'], {'SAC': {'desc': 'System Area Code', 'val': 25},
                                                 'SIC': {'desc': 'System Identification Code', 'val': 201}})
            self.assertEqual(packet[0]['I170'], {'CNF': {'desc': 'Confirmed Vs. Tentative Track', 'val': 0, 'meaning': 'Confirmed Track'},
                                                 'RAD': {'desc': 'Type of Sensor(s) Maintaining Track', 'val': 2, 'meaning': 'SSR/Mode S Track'},
                                                 'DOU': {'desc': 'Signals Level of Confidence in Plot to Track Association Process', 'val': 0, 'meaning': 'Normal confidence'},
                                                 'MAH': {'desc': 'Manoeuvre Detection in Horizontal Sense', 'val': 0, 'meaning': 'No horizontal man.sensed'},
                                                 'CDM': {'desc': 'Climbing / Descending Mode', 'val': 0, 'meaning': 'Maintaining'},
                                                 'FX': {'desc': 'Extension Indicator', 'val': 0, 'meaning': 'End of Data Item'},
                                                 'TRE': {'desc': 'Signal for End_of_Track', 'val': 0, 'meaning': 'Track still alive'},
                                                 'GHO': {'desc': 'Ghost Vs. True Target', 'val': 0, 'meaning': 'True target track'},
                                                 'SUP': {'desc': 'Track Maintained with Track Information from Neighbouring Node B on the Cluster, or Network', 'val': 0, 'meaning': 'No'},
                                                 'TCC': {'desc': 'Type of Plot Coordinate Transformation Mechanism:', 'val': 0, 'meaning': "Tracking performed in so-called 'Radar Plane', i.e. neither slant range correction nor stereographical projection was applied"},
                                                 'spare': {'desc': 'Spare bit(s) set to 0', 'val': 0, 'const': 0}})
            self.assertEqual(packet[0]['I200'], {'GSP': {'desc': 'Calculated Groundspeed', 'val': 0.12066650390625},
                                                 'HDG': {'desc': 'Calculated Heading', 'val': 124.002685546875}})
            self.assertEqual(packet[0]['I220'], {'AA': {'desc': 'Aircraft Address', 'val': 3958284}})

            # MBDATA contains actual BDS 4,0 register data (Selected Vertical Intention)
            # After 64-bit parser fix, correctly extracts: 0xc0780031bc0000 = 54175137758183424
            self.assertEqual(packet[0]['I250'][0], {'MBDATA': {'desc': 'Mode S Comm B Message Data', 'val': 54175137758183424},
                                                    'BDS1': {'desc': 'Comm B Data Buffer Store 1 Address', 'val': 4},
                                                    'BDS2': {'desc': 'Comm B Data Buffer Store 2 Address', 'val': 0}})

            self.assertEqual(packet[0]['I040'], {'THETA': {'desc': 'THETA', 'val': 340.13671875},
                                                 'RHO': {'desc': 'RHO', 'max': 256.0, 'val': 197.68359375}})
            self.assertEqual(packet[0]['I240'],
                             {'AI': {'desc': 'Aircraft Identification',
                                      'val': 'DLH65A  '}})
            self.assertEqual(packet[0]['I140'], {'ToD': {'desc': 'Time of Day', 'val': 27354.6015625, 'max': 86400.0}})
            self.assertEqual(packet[0]['I070'], {'V': {'desc': '', 'val': 0, 'meaning': 'Code validated'},
                                                 'G': {'desc': '', 'val': 0, 'meaning': 'Default'},
                                                 'L': {'desc': '', 'val': 0,
                                                       'meaning': 'Mode-3/A code derived from the reply of the transponder'},
                                                 'spare': {'desc': 'Spare bit(s) set to 0', 'const': 0, 'val': 0},
                                                 'MODE3A': {'desc': 'Mode-3/A Reply in Octal Representation', 'val': '1000'}})
            self.assertEqual(packet[0]['I161'], {'spare': {'desc': 'Spare bit(s) set to 0', 'val': 0, 'const': 0}, 'TRN': {'desc': 'Track Number', 'val': 3563}})
            self.assertEqual(packet[0]['I020'], {'TYP': {'desc': '', 'val': 5, 'meaning': 'Single ModeS Roll-Call'},
                                                 'SIM': {'desc': '', 'val': 0, 'meaning': 'Actual target report'},
                                                 'RDP': {'desc': '', 'val': 0, 'meaning': 'Report from RDP Chain 1'},
                                                 'SPI': {'desc': '', 'val': 0, 'meaning': 'Absence of SPI'},
                                                 'RAB': {'desc': '', 'val': 0,
                                                         'meaning': 'Report from aircraft transponder'},
                                                 'FX': {'desc': 'Extension Indicator', 'val': 0, 'meaning': 'End of Data Item'}})
            self.assertEqual(packet[0]['I090'], {'V': {'desc': '', 'val': 0, 'meaning': 'Code validated'},
                                                 'G': {'desc': '', 'val': 0, 'meaning': 'Default'},
                                                 'FL': {'desc': '', 'val': 330.0}})
            self.assertEqual(packet[0]['I230'], {'COM': {'desc': 'Communications Capability of the Transponder', 'val': 1,
                                                              'meaning': 'Comm. A and Comm. B capability'},
                                                 'STAT': {'desc': 'Flight Status', 'val': 0,
                                                          'meaning': 'No alert, no SPI, aircraft airborne'},
                                                 'SI': {'desc': 'SI/II Transponder Capability', 'val': 0,
                                                        'meaning': 'SI-Code Capable'},
                                                 'spare': {'desc': 'Spare bit(s) set to 0', 'const': 0, 'val': 0},
                                                 'MSSC': {'desc': 'Mode-S Specific Service Capability', 'val': 1,
                                                              'meaning': 'Yes'},
                                                 'ARC': {'desc': 'Altitude Reporting Capability', 'val': 1,
                                                         'meaning': '25 ft resolution'},
                                                 'AIC': {'desc': 'Aircraft Identification Capability', 'val': 1,
                                                         'meaning': 'Yes'},
                                                 'B1A': {'desc': 'BDS 1,0 Bit 16', 'val': 1},
                                                 'B1B': {'desc': 'BDS 1,0 Bits 37/40', 'val': 5}})

    def test_ParseCAT048_nodescription(self):
        sample_filename = asterix.get_sample_file('cat048.raw')
        with open(sample_filename, "rb") as f:
            data = f.read()
            packet = asterix.parse(data, verbose=False)
            self.maxDiff = None
            self.assertIsNotNone(packet)
            self.assertIsNotNone(packet[0])
            self.assertIs(len(packet), 1)
            self.assertTrue('I220' in packet[0])
            self.assertEqual(packet[0]['category'], 48)
            self.assertEqual(packet[0]['len'], 45)
            self.assertEqual(packet[0]['crc'], 'AB659C3E')
            self.assertTrue('ts' in packet[0])
            self.assertEqual(packet[0]['I220']['AA']['val'], 3958284)  # Aircraft Address as integer (0x3C660C = 3958284)
            self.assertEqual(packet[0]['I010'], {'SAC': {'val': 25},
                                                 'SIC': {'val': 201}})
            self.assertEqual(packet[0]['I170'], {'GHO': {'val': 0},
                                                 'TCC': {'val': 0},
                                                 'RAD': {'val': 2},
                                                 'spare': {'val': 0},
                                                 'TRE': {'val': 0},
                                                 'CDM': {'val': 0},
                                                 'CNF': {'val': 0},
                                                 'SUP': {'val': 0},
                                                 'FX': {'val': 0},
                                                 'DOU': {'val': 0},
                                                 'MAH': {'val': 0}})
            self.assertEqual(packet[0]['I200'], {'GSP': {'val': 0.12066650390625},
                                                 'HDG': {'val': 124.002685546875}})
            self.assertEqual(packet[0]['I220'], {'AA': {'val': 3958284}})
            # MBDATA: 64-bit BDS register data
            self.assertEqual(packet[0]['I250'][0], {'MBDATA': {'val': 54175137758183424},
                                                    'BDS1': {'val': 4},
                                                    'BDS2': {'val': 0}})
            self.assertEqual(packet[0]['I040'], {'THETA': {'desc': 'THETA', 'val': 340.13671875},
                                                 'RHO': {'desc': 'RHO', 'max': 256.0, 'val': 197.68359375}})
            self.assertEqual(packet[0]['I240'], {'AI': {'val': 'DLH65A  '}})
            self.assertEqual(packet[0]['I140'], {'ToD': {'val': 27354.6015625}})
            self.assertEqual(packet[0]['I070'], {'V': {'val': 0},
                                                 'G': {'val': 0},
                                                 'L': {'val': 0},
                                                 'spare': {'val': 0},
                                                 'MODE3A': {'val': '1000'}})
            self.assertEqual(packet[0]['I161'], {'spare': {'val': 0}, 'TRN': {'val': 3563}})
            self.assertEqual(packet[0]['I020'], {'SIM': {'val': 0},
                                                 'TYP': {'val': 5},
                                                 'RAB': {'val': 0},
                                                 'RDP': {'val': 0},
                                                 'FX': {'val': 0},
                                                 'SPI': {'val': 0}})
            self.assertEqual(packet[0]['I090'], {'V': {'val': 0},
                                                 'FL': {'val': 330.0},
                                                 'G': {'val': 0}})
            self.assertEqual(packet[0]['I230'], {'COM': {'val': 1},
                                                 'STAT': {'val': 0},
                                                 'SI': {'val': 0},
                                                 'spare': {'val': 0},
                                                 'MSSC': {'val': 1},
                                                 'ARC': {'val': 1},
                                                 'AIC': {'val': 1},
                                                 'B1A': {'val': 1},
                                                 'B1B': {'val': 5}})

    def test_ParseCAT062CAT065(self):
        self.maxDiff = None
        sample_filename = asterix.get_sample_file('cat062cat065.raw')
        with open(sample_filename, "rb") as f:
            data = f.read()
            packet = asterix.parse(data)
            self.assertIsNotNone(packet)
            self.assertIsNotNone(packet[0])
            self.assertIs(len(data), 195)
            self.assertIs(len(packet), 3)
            self.assertIs(packet[0]['category'], 62)
            self.assertIs(packet[0]['len'], 66)
            self.assertEqual(packet[0]['crc'], '943598B9')
            self.assertIs(packet[1]['category'], 62)
            self.assertIs(packet[1]['len'], 114)
            self.assertEqual(packet[1]['crc'], '0F55FD0E')
            self.assertIs(packet[2]['category'], 65)
            self.assertIs(packet[2]['len'], 9)
            self.assertEqual(packet[2]['crc'], 'ED9D3EB1')
            self.assertEqual(packet[0]['I220'], {'CRoC': {'val': -443.75, 'desc': 'Calculated Rate of Climb/Descent'}})
            self.assertEqual(packet[0]['I015'], {'SI': {'val': 4, 'desc': 'Service Identification'}})

            self.assertEqual(packet[0]['I290']['MDS'], {
                'MDS': {'val': 63.75, 'desc': 'Mode S Age', 'max': 63.75}})
            self.assertEqual(packet[0]['I290']['PSR'], {
                'PSR': {'val': 7.25, 'desc': 'PSR Age', 'max': 63.75}})
            self.assertEqual(packet[0]['I290']['SSR'], {
                'SSR': {'val': 0.0, 'desc': 'SSR Age', 'max': 63.75}})

            self.assertEqual(packet[0]['I135'], {
                'QNH': {'meaning': 'No QNH correction applied', 'val': 0, 'desc': ''},
                'CTB': {'max': 1500.0, 'min': -15.0, 'val': 157.0,
                         'desc': 'Calculated Track Barometric Altitude'}})
            self.assertEqual(packet[0]['I136'], {
                'MFL': {'max': 1500.0, 'min': -15.0, 'val': 157.0, 'desc': 'Measured Flight Level'}})
            self.assertEqual(packet[0]['I185'], {'VX': {'max': 8191.75, 'min': -8192.0, 'val': -51.25, 'desc': 'Velocity (X-component)'},
                                                 'VY': {'max': 8191.75, 'min': -8192.0, 'val': 170.0, 'desc': 'Velocity (Y-component)'}})
            self.assertEqual(packet[0]['I080'], {'STP': {'meaning': 'Default value', 'val': 0, 'desc': ''},
                                                 'MD5': {'meaning': 'No Mode 5 interrogation', 'val': 0, 'desc': ''},
                                                 'FPC': {'meaning': 'Not flight-plan correlated', 'val': 0, 'desc': ''},
                                                 'AMA': {'meaning': 'Track not resulting from amalgamation process',
                                                         'val': 0, 'desc': ''},
                                                 'CNF': {'meaning': 'Confirmed track', 'val': 0, 'desc': ''},
                                                 'TSE': {'meaning': 'Default value', 'val': 0, 'desc': ''},
                                                 'ME': {'meaning': 'Default value', 'val': 0, 'desc': ''},
                                                 'FX': {'meaning': 'End of Data Item', 'val': 0, 'desc': 'Extension Indicator'},
                                                 'CST': {'meaning': 'Default value', 'val': 0, 'desc': ''},
                                                 'PSR': {'meaning': 'Default value', 'val': 0, 'desc': ''}, 'MDS': {
                    'meaning': 'Age of the last received Mode S track update is higher than system dependent threshold',
                    'val': 1, 'desc': ''},
                                                 'MI': {'meaning': 'Default value', 'val': 0, 'desc': ''},
                                                 'SRC': {'meaning': 'Height from coverage', 'val': 4,
                                                         'desc': 'Source of Calculated Track Altitude for I062/130'},
                                                 'SIM': {'meaning': 'Actual track', 'val': 0, 'desc': ''},
                                                 'KOS': {'meaning': 'Background service used', 'val': 1, 'desc': ''},
                                                 'AFF': {'meaning': 'Default value', 'val': 0, 'desc': ''},
                                                 'MRH': {'meaning': 'Barometric altitude (Mode C) more reliable',
                                                         'val': 0, 'desc': 'Most Reliable Height'},
                                                 'MON': {'meaning': 'Multisensor track', 'val': 0, 'desc': ''},
                                                 'TSB': {'meaning': 'Default value', 'val': 0, 'desc': ''},
                                                 'SUC': {'meaning': 'Default value', 'val': 0, 'desc': ''},
                                                 'MD4': {'meaning': 'No Mode 4 interrogation', 'val': 0, 'desc': ''},
                                                 'SPI': {'meaning': 'Default value', 'val': 0, 'desc': ''}, 'ADS': {
                    'meaning': 'Age of the last received ADS-B track update is higher than system dependent threshold',
                    'val': 1, 'desc': ''},
                                                 'AAC': {'meaning': 'Default value', 'val': 0, 'desc': ''},
                                                 'SSR': {'meaning': 'Default value', 'val': 0, 'desc': ''}})
            self.assertEqual(packet[0]['I070'], {'TOTI': {'val': 30911.6640625, 'desc': 'Time Of Track Information'}})
            self.assertEqual(packet[0]['I100'], {'X': {'val': -239083.0, 'desc': 'X Coordinate'},
                                                 'Y': {'val': -106114.0, 'desc': 'Y Coordinate'}})
            self.assertEqual(packet[0]['I200'], {'TRANS': {'meaning': 'Constant course', 'val': 0, 'desc': 'Transversal Acceleration'},
                                                 'LONG': {'meaning': 'Decreasing groundspeed', 'val': 2,
                                                           'desc': 'Longitudinal Acceleration'},
                                                 'VERT': {'meaning': 'Descent', 'val': 2, 'desc': 'Transversal Acceleration'},
                                                 'ADF': {'meaning': 'No altitude discrepancy', 'val': 0,
                                                         'desc': 'Altitude Discrepancy Flag'},
                                                 'spare': {'const': 0, 'val': 0, 'desc': 'Spare bit(s) set to 0'}})
            self.assertEqual(packet[0]['I130'], {
                'CTGA': {'max': 150000.0, 'min': -1500.0, 'val': 43300.0, 'desc': 'Calculated Track Geometric Altitude'}})
            self.assertEqual(packet[0]['I060'], {'V': {'meaning': 'Code validated', 'val': 0, 'desc': 'Validated'},
                                                 'G': {'desc': 'Garbled', 'meaning': 'Default', 'val': 0},
                                                 'CH': {'meaning': 'No change', 'val': 0, 'desc': 'Change in Mode 3/A'},
                                                 'spare': {'const': 0, 'val': 0, 'desc': 'Spare bit(s) set to 0'},
                                                 'MODE3A': {'val': '4276',
                                                            'desc': 'Mode-3/A Reply in Octal Representation'}})

            self.assertEqual(packet[0]['I295']['MDA'], {'MDA': {'val': 0.0, 'desc': 'Mode 3/A Age', 'max': 63.75}})
            self.assertEqual(packet[0]['I295']['MFL'], {'MFL': {'val': 0.0, 'desc': 'Measured Flight Level Age', 'max': 63.75}})

            self.assertEqual(packet[0]['I010'], {'SAC': {'val': 25, 'desc': 'System Area Code'},
                                                 'SIC': {'val': 100, 'desc': 'System Identification Code'}})

            self.assertEqual(packet[0]['I340']['TYP'], {
                'TYP': {'val': 2, 'meaning': 'Single SSR detection', 'desc': 'Report Type'},
                'SIM': {'val': 0, 'meaning': 'Actual target report', 'desc': ''},
                'RAB': {'val': 0, 'meaning': 'Report from target transponder', 'desc': ''},
                'TST': {'val': 0, 'meaning': 'Real target report', 'desc': ''},
                'spare': {'val': 0, 'desc': 'Spare bit(s) set to 0', 'const': 0}})

            self.assertEqual(packet[0]['I340']['SID'], {
                'SAC': {'val': 25, 'desc': 'System Area Code'},
                'SIC': {'val': 13, 'desc': 'System Identification Code'}})

            self.assertEqual(packet[0]['I340']['MDC'], {
                'V': {'val': 0, 'meaning': 'Code validated', 'desc': 'Validated'},
                'G': {'val': 0, 'meaning': 'Default', 'desc': 'Garbled'},
                'LMC': {'max': 1270.0, 'val': 157.0, 'min': -12.0, 'desc': 'Last Measured Mode C Code'}})

            self.assertEqual(packet[0]['I340']['MDA'], {
                'V': {'val': 0, 'meaning': 'Code validated', 'desc': 'Validated'},
                'G': {'val': 0, 'meaning': 'Default', 'desc': 'Garbled'},
                'L': {'val': 0, 'meaning': 'Mode 3/A code as derived from the reply of the transponder',
                      'desc': ''},
                'spare': {'val': 0, 'desc': 'Spare bit(s) set to 0', 'const': 0},
                'MODE3A': {'val': '4276',
                           'desc': 'Mode-3/A Reply in Octal Representation'}})

            self.assertEqual(packet[0]['I340']['POS'], {
                'RHO': {'max': 256.0, 'val': 186.6875, 'desc': 'Measured Distance'},
                'THETA': {'val': 259.453125, 'desc': 'Measured Azimuth'}})

            self.assertEqual(packet[0]['I105'], {
                'LAT': {'val': 44.73441302776337,
                        'desc': 'Latitude', 'max': 90.0, 'min': -90.0},
                'LON': {
                'val': 13.0415278673172,
                'desc': 'Longitude', 'max': 180.0, 'min': -180.0}})
            self.assertEqual(packet[0]['I040'], {'TN': {'val': 4980, 'desc': 'Track Number'}})
            self.assertEqual(packet[0]['I210'],
                                 {'AX': {'val': 0.0, 'desc': ''}, 'AY': {'val': 0.0, 'desc': ''}})
            self.assertEqual(packet[1]['I220'], {'CRoC': {'val': 0.0, 'desc': 'Calculated Rate of Climb/Descent'}})
            self.assertEqual(packet[1]['I015'], {'SI': {'val': 4, 'desc': 'Service Identification'}})

            self.assertEqual(packet[1]['I290']['MDS'], {
                'MDS': {'val': 0.0, 'desc': 'Mode S Age', 'max': 63.75}})
            self.assertEqual(packet[1]['I290']['SSR'], {
                'SSR': {'val': 0.0, 'desc': 'SSR Age', 'max': 63.75}})

            self.assertEqual(packet[1]['I135'], {
                'QNH': {'meaning': 'No QNH correction applied', 'val': 0, 'desc': ''},
                'CTB': {'max': 1500.0, 'min': -15.0, 'val': 350.0,
                         'desc': 'Calculated Track Barometric Altitude'}})
            self.assertEqual(packet[1]['I136'], {
                 'MFL': {'max': 1500.0, 'min': -15.0, 'val': 350.0,
                        'desc': 'Measured Flight Level'}})
            self.assertEqual(packet[1]['I185'], {'VX': {'max': 8191.75, 'min': -8192.0, 'val': 141.5, 'desc': 'Velocity (X-component)'},
                                                 'VY': {'max': 8191.75, 'min': -8192.0, 'val': -170.75,
                                                        'desc': 'Velocity (Y-component)'}})
            self.assertEqual(packet[1]['I080'], {'STP': {'meaning': 'Default value', 'val': 0, 'desc': ''},
                                                     'MD5': {'meaning': 'No Mode 5 interrogation', 'val': 0,
                                                             'desc': ''},
                                                     'FPC': {'meaning': 'Flight plan correlated', 'val': 1,
                                                             'desc': ''},
                                                     'AMA': {
                                                         'meaning': 'Track not resulting from amalgamation process',
                                                         'val': 0, 'desc': ''},
                                                     'CNF': {'meaning': 'Confirmed track', 'val': 0, 'desc': ''},
                                                     'TSE': {'meaning': 'Default value', 'val': 0, 'desc': ''},
                                                     'ME': {'meaning': 'Default value', 'val': 0, 'desc': ''},
                                                     'FX': {'meaning': 'End of Data Item', 'val': 0, 'desc': 'Extension Indicator'},
                                                     'CST': {'meaning': 'Default value', 'val': 0, 'desc': ''},
                                                     'PSR': {'meaning': 'Default value', 'val': 0, 'desc': ''},
                                                     'MDS': {'meaning': 'Default value', 'val': 0, 'desc': ''},
                                                     'MI': {'meaning': 'Default value', 'val': 0, 'desc': ''},
                                                     'SRC': {'meaning': 'Triangulation', 'val': 3,
                                                             'desc': 'Source of Calculated Track Altitude for I062/130'},
                                                     'SIM': {'meaning': 'Actual track', 'val': 0, 'desc': ''},
                                                     'KOS': {'meaning': 'Background service used', 'val': 1,
                                                             'desc': ''},
                                                     'AFF': {'meaning': 'Default value', 'val': 0, 'desc': ''},
                                                     'MRH': {
                                                         'meaning': 'Barometric altitude (Mode C) more reliable',
                                                         'val': 0, 'desc': 'Most Reliable Height'},
                                                     'MON': {'meaning': 'Multisensor track', 'val': 0, 'desc': ''},
                                                     'TSB': {'meaning': 'Default value', 'val': 0, 'desc': ''},
                                                     'SUC': {'meaning': 'Default value', 'val': 0, 'desc': ''},
                                                     'MD4': {'meaning': 'No Mode 4 interrogation', 'val': 0,
                                                             'desc': ''},
                                                     'SPI': {'meaning': 'Default value', 'val': 0, 'desc': ''},
                                                     'ADS': {
                                                         'meaning': 'Age of the last received ADS-B track update is higher than system dependent threshold',
                                                         'val': 1, 'desc': ''},
                                                     'AAC': {'meaning': 'Default value', 'val': 0, 'desc': ''},
                                                     'SSR': {'meaning': 'Default value', 'val': 0, 'desc': ''}})
            self.assertEqual(packet[1]['I070'], {'TOTI': {'val': 30911.828125, 'desc': 'Time Of Track Information'}})
            self.assertEqual(packet[1]['I100'], {'X': {'val': -72564.5, 'desc': 'X Coordinate'},
                                                     'Y': {'val': -36106.5, 'desc': 'Y Coordinate'}})
            self.assertEqual(packet[1]['I200'], {'TRANS': {'meaning': 'Constant course', 'val': 0, 'desc': 'Transversal Acceleration'},
                                                 'LONG': {'meaning': 'Constant groundspeed', 'val': 0,
                                                           'desc': 'Longitudinal Acceleration'},
                                                 'VERT': {'meaning': 'Level', 'val': 0, 'desc': 'Transversal Acceleration'},
                                                 'ADF': {'meaning': 'No altitude discrepancy', 'val': 0,
                                                         'desc': 'Altitude Discrepancy Flag'},
                                                 'spare': {'const': 0, 'val': 0, 'desc': 'Spare bit(s) set to 0'}})
            self.assertEqual(packet[1]['I130'], {
                    'CTGA': {'max': 150000.0, 'min': -1500.0, 'val': 35312.5, 'desc': 'Calculated Track Geometric Altitude'}})
            self.assertEqual(packet[1]['I060'],
                             {'V': {'desc': 'Validated', 'meaning': 'Code validated',
                                    'val': 0},
                              'G': {'desc': 'Garbled', 'meaning': 'Default', 'val': 0},
                              'CH': {'meaning': 'No change', 'val': 0, 'desc': 'Change in Mode 3/A'},
                              'spare': {'const': 0, 'val': 0, 'desc': 'Spare bit(s) set to 0'},
                              'MODE3A': {'val': '2535',
                                         'desc': 'Mode-3/A Reply in Octal Representation'}})

            self.assertEqual(packet[1]['I295']['MFL'], {'MFL': {'val': 0.0, 'desc': 'Measured Flight Level Age', 'max': 63.75}})

            self.assertEqual(packet[1]['I390']['DEP'], {'DEP': {'desc': 'Departure Airport', 'val': 'EDDL'}})
            self.assertEqual(packet[1]['I390']['TAC'], {'TAC': {'desc': 'Type of Aircraft', 'val': 'B738'}})
            self.assertEqual(packet[1]['I390']['DST'], {'DST': {'desc': 'Destination Airport', 'val': 'HELX'}})

            self.assertEqual(packet[1]['I390']['IFI'], {
                    'TYP': {'meaning': 'Unit 1 internal flight number', 'desc': '', 'val': 1},
                    'spare': {'const': 0, 'desc': 'Spare bit(s) set to 0', 'val': 0},
                    'NBR': {'desc': 'Number from 0 to 99 999 999', 'val': 29233709}})

            self.assertEqual(packet[1]['I390']['RDS'], {'NU1': {'desc': 'First Number', 'val': ' '},
                                                        'NU2': {'desc': 'Second Number', 'val': ''},
                                                        'LTR': {'desc': 'Letter', 'val': ' '}})

            self.assertEqual(packet[1]['I390']['WTC'],
                                 {'WTC': {'desc': 'Wake Turbulence Category', 'val': 'M'}})

            self.assertEqual(packet[1]['I390']['CS'], {'CS': {'desc': 'Callsign', 'val': 'SXD4723'}})
            self.assertEqual(packet[1]['I390']['TAG'], {
                    'SIC': {'desc': 'System Identification Code', 'val': 100},
                    'SAC': {'desc': 'System Area Code', 'val': 25}})

            self.assertEqual(packet[1]['I390']['FCT'], {
                'GATOAT': {'meaning': 'General Air Traffic', 'desc': '', 'val': 1},
                'FR1FR2': {'meaning': 'Instrument Flight Rules', 'desc': '', 'val': 0},
                'RVSM': {'meaning': 'Approved', 'desc': '', 'val': 1},
                'HPR': {'meaning': 'Normal Priority Flight', 'desc': '', 'val': 0},
                'spare': {'const': 0, 'desc': 'Spare bit(s) set to 0', 'val': 0}})

            self.assertEqual(packet[1]['I390']['CFL'],
                                 {'CFL': {'desc': 'Current Cleared Flight Level', 'val': 350.0, 'max': 1500.0}})
            self.assertEqual(packet[1]['I010'], {'SAC': {'val': 25, 'desc': 'System Area Code'},
                                                 'SIC': {'val': 100, 'desc': 'System Identification Code'}})

            self.assertEqual(packet[1]['I340']['TYP'], {
                    'TYP': {'desc': 'Report Type', 'meaning': 'Single ModeS Roll-Call', 'val': 5},
                    'SIM': {'desc': '', 'meaning': 'Actual target report', 'val': 0},
                    'RAB': {'desc': '', 'meaning': 'Report from target transponder', 'val': 0},
                    'TST': {'desc': '', 'meaning': 'Real target report', 'val': 0},
                    'spare': {'desc': 'Spare bit(s) set to 0', 'val': 0, 'const': 0}})
            self.assertEqual(packet[1]['I340']['POS'], {
                'RHO': {'desc': 'Measured Distance', 'val': 93.1953125, 'max': 256.0},
                'THETA': {'desc': 'Measured Azimuth', 'val': 271.4666748046875}})
            self.assertEqual(packet[1]['I340']['MDA'], {
                    'V': {'desc': 'Validated', 'meaning': 'Code validated', 'val': 0},
                    'G': {'desc': 'Garbled', 'meaning': 'Default', 'val': 0},
                    'L': {'desc': '', 'meaning': 'Mode 3/A code as derived from the reply of the transponder',
                          'val': 0},
                    'spare': {'desc': 'Spare bit(s) set to 0', 'val': 0, 'const': 0},
                    'MODE3A': {'desc': 'Mode-3/A Reply in Octal Representation',
                               'val': '2535'}})
            self.assertEqual(packet[1]['I340']['MDC'], {
                'V': {'desc': 'Validated', 'meaning': 'Code validated', 'val': 0},
                'G': {'desc': 'Garbled', 'meaning': 'Default', 'val': 0},
                'LMC': {'min': -12.0, 'desc': 'Last Measured Mode C Code', 'val': 350.0, 'max': 1270.0}})
            self.assertEqual(packet[1]['I340']['SID'], {
                    'SIC': {'desc': 'System Identification Code', 'val': 13},
                    'SAC': {'desc': 'System Area Code', 'val': 25}})

            self.assertEqual(packet[1]['I380']['COM'], {
                'COM': {'val': 1, 'meaning': 'Comm. A and Comm. B capability',
                        'desc': 'Communications Capability of the Transponder'},
                'STAT': {'val': 0, 'meaning': 'No alert, no SPI, aircraft airborne', 'desc': 'Flight Status'},
                'spare': {'val': 0, 'const': 0, 'desc': 'Spare bit(s) set to 0'},
                'SSC': {'val': 1, 'meaning': 'Yes', 'desc': 'Specific Service Capability'},
                'ARC': {'val': 1, 'meaning': '25 ft resolution', 'desc': 'Altitude Reporting Capability'},
                'AIC': {'val': 1, 'meaning': 'Yes', 'desc': 'Aircraft Identification Capability'},
                'B1A': {'val': 1, 'desc': 'BDS 1,0 Bit 16'},
                'B1B': {'val': 6, 'desc': 'BDS BDS 1,0 Bits 37/40'}})
            self.assertEqual(packet[1]['I380']['ADR'], {'ADR': {'val': 3934805, 'desc': 'Target Address'}})
            self.assertEqual(packet[1]['I380']['ID'],
                             {'ID': {'val': 'SXD4723 ', 'desc': 'Target Identification'}})

            self.assertEqual(packet[1]['I105'], {'LAT': {'val': 45.40080785751343,
                                                             'desc': 'Latitude', 'max': 90.0, 'min': -90.0},
                                                      'LON': {
            'val': 15.13318419456482,
            'desc': 'Longitude', 'max': 180.0, 'min': -180.0}})
            self.assertEqual(packet[1]['I040'], {'TN': {'val': 7977, 'desc': 'Track Number'}})
            self.assertEqual(packet[1]['I210'],
                                 {'AX': {'val': 0.0, 'desc': ''}, 'AY': {'val': 0.0, 'desc': ''}})
            self.assertEqual(packet[2]['I015'], {'SI': {'val': 4, 'desc': 'Service Identification'}})
            self.assertEqual(packet[2]['I020'], {'BN': {'val': 24, 'desc': 'Batch Number'}})
            self.assertEqual(packet[2]['I010'], {'SAC': {'val': 25, 'desc': 'System Area Code'},
                                                 'SIC': {'val': 100, 'desc': 'System Identification Code'}})
            self.assertEqual(packet[2]['I030'], {'ToM': {'val': 30913.0546875, 'desc': 'Time of Message'}})
            self.assertEqual(packet[2]['I000'],
                             {'MT': {'meaning': 'End of Batch', 'val': 2, 'desc': 'Message Type'}})


def main():
    unittest.main()


if __name__ == '__main__':
    main()
