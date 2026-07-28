using System.Text;
using System.IO.Ports;
using System.Threading;
using System.Globalization;
using System;

class MERCURY
{
	const byte STX = 0x02;   // Start Of Text
	const byte ETX = 0x03;   // End of Text
	const byte HEADER_OFFSET = 0;
	const byte ADDRESS_HIGH_OFFSET = 1;
	const byte ADDRESS_LOW_OFFSET = 2;
	const byte LEN_OFFSET = 3;
	const byte CMND_OFFSET = 6;
	const byte SUB_CMND_OFFSET = 7;

	byte[] msg = null;
	SerialPort _serialPort;

	public MERCURY(string port, int baudrate)
	{
		_serialPort = new SerialPort(port, baudrate, Parity.None, 8, StopBits.One);
		_serialPort.Open();
		_serialPort.ReadTimeout = 5000;
	}

	public void close() 
	{
		_serialPort.Close();
	}

	private byte xorCRC(byte[] msg)
	{
		int i;

		byte crcValue = 0;

		for (i = 3; i < msg.Length - 3; i++)
		{
			crcValue ^= msg[i];
		}

		return (crcValue);
	}
	
	private int Send(byte cmd, byte[] parameters)
	{
		byte computedCRC = 0;

		msg = new byte[7 + parameters.Length + 3];
		if (parameters.Length > 999)
		{
			return (-1);
		}
		msg[HEADER_OFFSET]       = (0x80 | STX);
		msg[ADDRESS_HIGH_OFFSET] = (0x80 | '0');
		msg[ADDRESS_LOW_OFFSET]  = (0x80 | '1');
		msg[LEN_OFFSET]          = (byte)parameters.Length.ToString("000")[0];
		msg[LEN_OFFSET + 1]      = (byte)parameters.Length.ToString("000")[1];
		msg[LEN_OFFSET + 2]      = (byte)parameters.Length.ToString("000")[2];
		msg[CMND_OFFSET]         = cmd;
		parameters.CopyTo(msg, CMND_OFFSET + 1);
		computedCRC = xorCRC(msg);
		msg[6 + parameters.Length+1] = (byte)computedCRC.ToString("X2")[0];
		msg[6 + parameters.Length + 2] = (byte)computedCRC.ToString("X2")[1];
		msg[6 + parameters.Length + 3] = ETX;
		_serialPort.Write(msg, 0, msg.Length);
		return (0);
	}

	private int Receive(ref byte[] reply)
	{
		byte[] msg = new byte[7];
		int address, length, rxBytes = 0;   
		byte computedCRC = 0;
		try
		{
			do
			{
				rxBytes += _serialPort.Read(msg, rxBytes, msg.Length - rxBytes);
			} while (rxBytes < msg.Length);

			if (msg[HEADER_OFFSET] != STX)
			{
				return (-1);
			}
			if ((msg[ADDRESS_HIGH_OFFSET] < '0') || (msg[ADDRESS_HIGH_OFFSET] > '9'))
			{
				return (-2);
			}
			address = (msg[ADDRESS_HIGH_OFFSET] - '0') * 10;
			if ((msg[ADDRESS_LOW_OFFSET + 1] < '0') || (msg[ADDRESS_LOW_OFFSET] > '9'))
			{
				return (-3);
			}
			address += (msg[ADDRESS_LOW_OFFSET] - '0');

			if ((msg[LEN_OFFSET] < '0') || (msg[LEN_OFFSET] > '9'))
			{
				return (-4);
			}
			length = 100 * (msg[LEN_OFFSET] - '0');
			if ((msg[LEN_OFFSET + 1] < '0') || (msg[LEN_OFFSET + 1] > '9'))
			{
				return (-5);
			}
			length += 10 * (msg[LEN_OFFSET + 1] - '0');
			if ((msg[LEN_OFFSET + 2] < '0') || (msg[LEN_OFFSET + 2] > '9'))
			{
				return (-6);
			}
			length += (msg[LEN_OFFSET + 2] - '0');

			System.Array.Resize(ref msg, msg.Length + length + 3);

			do
			{
				rxBytes += _serialPort.Read(msg, rxBytes, msg.Length - rxBytes);
			} while (rxBytes < msg.Length);

			computedCRC = xorCRC(msg);
			if ((msg[msg.Length - 1 - 2] != (byte)computedCRC.ToString("X2")[0]) ||
				(msg[msg.Length - 1 - 1] != (byte)computedCRC.ToString("X2")[1])
			   )
			{
				return (-7);
			}
			reply = new byte[length];
			System.Array.Copy(msg, 7, reply, 0, length);
		}
		catch
		{
			return (-8);
		}
		return (0);
	}

	public void SetHV(bool enable, int voltage, int current)
	{
		byte cmd = (byte)'S';
		string parameters;

		if (enable)
		{
			parameters = "H" + "E" + "1" + "V" + voltage.ToString() + "I" + current.ToString();
		}
		else
		{
			parameters = "H" + "E" + "0" + "V" + voltage.ToString() + "I" + current.ToString();
		}

		Send(cmd, Encoding.ASCII.GetBytes(parameters));
		//Receive(cmd);
	}

	public void SetGM1HVEnable(bool gm1HVEnable)
	{
		byte cmd = (byte)'D';
		string parameters;

		if (gm1HVEnable)
		{
			parameters = "o" + "e" + "1";
		}
		else
		{
			parameters = "o" + "e" + "0";
		}

		Send(cmd, Encoding.ASCII.GetBytes(parameters));
	}

	public void SetGeiger(bool gm1HVEnable, int gm1QuenchingTime, int gm2QuenchingTime)
	{
		byte cmd = (byte)'S';
		string parameters; 
		
		if (gm1HVEnable)
		{
			parameters = "o" + "E" + "1" + "Q" + gm1QuenchingTime.ToString() + "Q" + gm2QuenchingTime.ToString();
		}
		else
		{
			parameters = "o" + "E" + "0" + "Q" + gm1QuenchingTime.ToString() + "Q" + gm2QuenchingTime.ToString();
		}

		Send(cmd, Encoding.ASCII.GetBytes(parameters));
	}

	public void SetEventDeadTime(int gm1QuenchingTime, int gm2QuenchingTime)
	{
		byte cmd = (byte)'S';
		string parameters;

		parameters = "e" + "T" + gm1QuenchingTime.ToString() + "T" + gm2QuenchingTime.ToString();

		Send(cmd, Encoding.ASCII.GetBytes(parameters));
	}

	public void SetDeadTime(int gm1QuenchingTime, int gm2QuenchingTime)
	{
		byte cmd = (byte)'S';
		string parameters;
			
		parameters = "d" + "T" + gm1QuenchingTime.ToString() + "T" + gm2QuenchingTime.ToString();

		Send(cmd, Encoding.ASCII.GetBytes(parameters));
	}

	public void SetGAMMAsGM1(int numerator, int denominator)
	{
		byte cmd = (byte)'S';
		string parameters;

		parameters = "g" + "N" + numerator.ToString() + "D" + denominator.ToString();

		Send(cmd, Encoding.ASCII.GetBytes(parameters));
	}

	public void SetGAMMAsGM2(int numerator, int denominator)
	{
		byte cmd = (byte)'S';
		string parameters;

		parameters = "G" + "N" + numerator.ToString() + "D" + denominator.ToString();

		Send(cmd, Encoding.ASCII.GetBytes(parameters));
	}

	public void SetZoneDescriptorGM1(ulong[] tapDepth, ulong[] threshold)
	{
		byte cmd = (byte)'S';
		string parameters;

		parameters = "t" + "H" + tapDepth[0].ToString()  +
						   "H" + threshold[0].ToString() +
						   "H" + threshold[1].ToString() +
						   "H" + tapDepth[1].ToString()  +
						   "H" + threshold[2].ToString() +
						   "H" + threshold[3].ToString() +
						   "H" + tapDepth[2].ToString()  +
						   "H" + threshold[4].ToString() +
						   "H" + threshold[5].ToString() +
						   "H" + tapDepth[3].ToString()  +
						   "H" + threshold[6].ToString() +
						   "H" + threshold[7].ToString();
		Send(cmd, Encoding.ASCII.GetBytes(parameters));
	}
    
	public void SetZoneDescriptorGM2(ulong[] tapDepth, ulong[] threshold)
	{
		byte cmd = (byte)'S';
		string parameters;

		parameters = "T" + "H" + tapDepth[0].ToString()   +
						   "H" + threshold[0].ToString()  +
						   "H" + threshold[1].ToString()  +
						   "H" + tapDepth[1].ToString()   +
						   "H" + threshold[2].ToString()  +
						   "H" + threshold[3].ToString()  +
						   "H" + tapDepth[2].ToString()   +
						   "H" + threshold[4].ToString()  +
						   "H" + threshold[5].ToString()  +
						   "H" + tapDepth[3].ToString()   +
						   "H" + threshold[6].ToString()  +
						   "H" + threshold[7].ToString()  +
						   "H" + tapDepth[4].ToString()   +
						   "H" + threshold[8].ToString()  +
						   "H" + threshold[9].ToString()  +
						   "H" + tapDepth[5].ToString()   +
						   "H" + threshold[10].ToString() +
						   "H" + threshold[11].ToString() +
						   "H" + tapDepth[6].ToString()   +
						   "H" + threshold[12].ToString() +
						   "H" + threshold[13].ToString() +
						   "H" + tapDepth[7].ToString()   +
						   "H" + threshold[14].ToString() +
						   "H" + threshold[15].ToString();
		Send(cmd, Encoding.ASCII.GetBytes(parameters));
	}

	public void SetTubeSwitchingZone(ulong lowThreshold, ulong highThreshold)
	{
		byte cmd = (byte)'S';
		string parameters;

		parameters = "s" + "H" + lowThreshold.ToString() +
						   "H" + highThreshold.ToString();
		Send(cmd, Encoding.ASCII.GetBytes(parameters));
	}

	public void SaveConfiguration()
	{
		byte cmd = (byte)'S';
		string parameters;

		parameters = "M";
		Send(cmd, Encoding.ASCII.GetBytes(parameters));
	}

	public void EraseConfiguration()
	{
		byte cmd = (byte)'S';
		string parameters;

		parameters = "E";
		Send(cmd, Encoding.ASCII.GetBytes(parameters));
	}

	public void ResetGeigerMuller()
	{
		byte cmd = (byte)'R';
		string parameters;

		parameters = "R";
		Send(cmd, Encoding.ASCII.GetBytes(parameters));
	}

	public (bool is_new, float doseRate) GetDoseRateGM1()
	{
		byte cmd = (byte)'D';
		string parameters;
		byte[] reply = null;
		string doseRateString = "";
		float doseRate = float.NaN;

		_serialPort.ReadExisting();
		parameters = "i";
		Send(cmd, Encoding.ASCII.GetBytes(parameters));
		if (Receive(ref reply) < 0)
		{
			throw new System.Exception("Communication Error");
		}

		try
		{
			doseRateString = System.Text.Encoding.ASCII.GetString(reply);
		}
		catch (Exception ex)
		{
			doseRate = float.NaN;
		}
		if (doseRateString.Substring(0, 2) == "ik")
		{
			if (doseRateString.Substring(2) == "99999999.9999")
			{
				doseRate = float.NaN;
			}
			else
			{
				doseRate = float.Parse(doseRateString.Substring(2), CultureInfo.InvariantCulture) * 1000.0f;
			}
			return (true, doseRate);

		}
		else if (doseRateString.Substring(0, 2) == "iK") 
		{
			// The sample has not been refreshed yet
			if (doseRateString.Substring(2) == "99999999.9999")
			{
				doseRate = float.NaN;
			}
			else
			{
				doseRate = float.Parse(doseRateString.Substring(2), CultureInfo.InvariantCulture) * 1000.0f;
			}

			return (false, doseRate);

		} else
		{

			throw new System.Exception("Communication Error");
		}

	}

	public byte[] GetAverageDoseRateGM1()
	{
		byte cmd = (byte)'D';
		string parameters;
		byte[] reply = null;

		_serialPort.ReadExisting();
		parameters = "a";
		Send(cmd, Encoding.ASCII.GetBytes(parameters));
		if (Receive(ref reply) < 0)
		{
			throw new System.Exception("Communication Error");
		}
		return reply;
	}

	public (bool is_new, float doseRate) GetDoseRateGM2()
	{
		byte cmd = (byte)'D';
		string parameters;
		byte[] reply = null;
		string doseRateString = "";
		float doseRate = float.NaN;

		_serialPort.ReadExisting();
		parameters = "I";
		Send(cmd, Encoding.ASCII.GetBytes(parameters));
		if (Receive(ref reply) < 0)
		{
			throw new System.Exception("Communication Error");
		}

		try
		{
			doseRateString = System.Text.Encoding.ASCII.GetString(reply);
		}
		catch (Exception ex)
		{
			doseRate = float.NaN;
		}
		if (doseRateString.Substring(0, 2) == "Ik")
		{
			if (doseRateString.Substring(2) == "99999999.9999")
			{
				doseRate = float.NaN;
			}
			else
			{
				doseRate = float.Parse(doseRateString.Substring(2), CultureInfo.InvariantCulture) * 1000.0f;
			}
			return (true, doseRate);

		}
		else if (doseRateString.Substring(0, 2) == "IK")
		{
			// The sample has not been refreshed yet
			if (doseRateString.Substring(2) == "99999999.9999")
			{
				doseRate = float.NaN;
			}
			else
			{
				doseRate = float.Parse(doseRateString.Substring(2), CultureInfo.InvariantCulture) * 1000.0f;
			}

			return (false, doseRate);

		}
		else
		{

			throw new System.Exception("Communication Error");
		}

	}

	public byte[] GetAverageDoseRateGM2()
	{
		byte cmd = (byte)'D';
		string parameters;
		byte[] reply = null;

		_serialPort.ReadExisting();
		parameters = "A";
		Send(cmd, Encoding.ASCII.GetBytes(parameters));
		if (Receive(ref reply) < 0)
		{
			throw new System.Exception("Communication Error");
		}
		return reply;
	}

	public byte[] GetDoseRate()
	{
		byte cmd = (byte)'M';
		string parameters;
		byte[] reply = null;

		_serialPort.ReadExisting();
		parameters = "I";
		Send(cmd, Encoding.ASCII.GetBytes(parameters));
		if (Receive(ref reply) < 0)
		{
			throw new System.Exception("Communication Error");
		}
		return reply;
	}

	public byte[] GetAllDoseRatesGM1 ()
	{
		byte cmd = (byte)'D';
		string parameters;
		byte[] reply = null;
		int error;

		_serialPort.ReadExisting();
		parameters = "c";
		Send(cmd, Encoding.ASCII.GetBytes(parameters));
		error = Receive(ref reply);
		if (error < 0)
		{
			throw new System.Exception("Communication Error");
		}
		return reply;
	}

	public byte[] GetAllDoseRatesGM2()
	{
		byte cmd = (byte)'D';
		string parameters;
		byte[] reply = null;
		int error;

		_serialPort.ReadExisting();
		parameters = "C";
		Send(cmd, Encoding.ASCII.GetBytes(parameters));
		error = Receive(ref reply);
		if (error < 0)
		{
			throw new System.Exception("Communication Error");
		}
		return reply;
	}

	public (uint voltage, uint current, uint ovc) GetHV()
	{
		byte cmd = (byte)'G';
		string parameters;
		byte[] reply = null;
		uint voltage, current, ovc;
		int error;
		
		_serialPort.ReadExisting();
		parameters = "H";
		Send(cmd, Encoding.ASCII.GetBytes(parameters));
		error = Receive(ref reply);
		if (error < 0)
		{
			throw new System.Exception("Communication Error");
		}
		string hvString = System.Text.Encoding.ASCII.GetString(reply);
		voltage = uint.Parse(hvString.Substring(2, 4), CultureInfo.InvariantCulture);
		current = uint.Parse(hvString.Substring(7, 4), CultureInfo.InvariantCulture);
		ovc = uint.Parse(hvString.Substring(12, 1), CultureInfo.InvariantCulture);
		return (voltage, current, ovc);
	}

	public byte[] GetStatus()
	{
		byte cmd = (byte)'G';
		string parameters;
		byte[] reply = null;

		_serialPort.ReadExisting();
		parameters = "e";
		Send(cmd, Encoding.ASCII.GetBytes(parameters));
		Thread.Sleep(100);
		if (Receive(ref reply) < 0)
		{
			throw new System.Exception("Communication Error");
		}
		return reply;
	}

	public void SetDateTime(string datetime)
	{
		byte cmd = (byte)'S';
		string parameters;
		byte[] reply = null;

		_serialPort.ReadExisting();
		parameters = "D";
		parameters += datetime;
		Send(cmd, Encoding.ASCII.GetBytes(parameters));
		Thread.Sleep(100);
	}

	public DateTime GetDateTime()
	{
		byte cmd = (byte)'G';
		string parameters;
		byte[] reply = null;
		string format = "dd/MM/yyyy H:mm:ss";

		_serialPort.ReadExisting();
		parameters = "D";
		Send(cmd, Encoding.ASCII.GetBytes(parameters));
		Thread.Sleep(100);
		if (Receive(ref reply) < 0)
		{
			throw new System.Exception("Communication Error");
		}
		string dateTimeStr = System.Text.Encoding.ASCII.GetString(reply).Substring(1, 19);

		DateTime datetime = DateTime.ParseExact(dateTimeStr, format, System.Globalization.CultureInfo.InvariantCulture);

		return datetime;

	}

	public float GetTemperature()
	{
		byte cmd = (byte)'G';
		string parameters;
		byte[] reply = null;
		string temperatureString;
		float temperature = 0.0F;

		_serialPort.ReadExisting();
		parameters = "T";
		Send(cmd, Encoding.ASCII.GetBytes(parameters));
		Thread.Sleep(100);
		if (Receive(ref reply) < 0)
		{
			throw new System.Exception("Communication Error");
		}
		temperatureString = System.Text.Encoding.ASCII.GetString(reply);
		temperature = float.Parse(temperatureString.Substring(1), CultureInfo.InvariantCulture);
		return temperature;
	}

	public string GetFirmwareRelease()
	{
		byte cmd = (byte)'G';
		string parameters;
		byte[] reply = null;
		string firmwareString;
		float temperature = 0.0F;

		_serialPort.ReadExisting();
		parameters = "F";
		Send(cmd, Encoding.ASCII.GetBytes(parameters));
		Thread.Sleep(100);
		if (Receive(ref reply) < 0)
		{
			throw new System.Exception("Communication Error");
		}
		firmwareString = System.Text.Encoding.ASCII.GetString(reply);
		return firmwareString.Substring(1);
	}

	public byte[] Reset()
	{
		byte cmd = (byte)'R';
		string parameters;
		byte[] reply = null;

		_serialPort.ReadExisting();
		parameters = "R";
		Send(cmd, Encoding.ASCII.GetBytes(parameters));
		Thread.Sleep(100);
		if (Receive(ref reply) < 0)
		{
			throw new System.Exception("Communication Error");
		}
		return reply;
	}

}

