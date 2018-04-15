import java.text.DecimalFormat;

/**
 * Simple example for SBE-4 seabird conductiviry sensor
 *
 * @author Peter Valencic
 */
public class ConductivitySalinity {

    public static void main(String[] args) {
        double condutctivity = condutctivity(9.63516, 15.0);
        double salinity = salinity(15, condutctivity * 10, 1);
        System.out.println("Conductivity [S/m]:  " + new DecimalFormat("##.#######################").format(condutctivity));
        System.out.println("Salinity [PSU]:  " + new DecimalFormat("##.#######################").format(salinity));
    }

    /**
     * function for salinity calulcation
     *
     * @param t temperature °C
     * @param c conductivity ms/cm (S/m)*10
     * @param p pressure dBar
     * @return PSU
     */
    public static double salinity(double t, double c, double p) {
        double a0 = 0.008;
        double a1 = -0.1692;
        double a2 = 25.3851;
        double a3 = 14.0941;
        double a4 = -7.0261;
        double a5 = 2.7081;
        double b0 = 0.0005;
        double b1 = -0.0056;
        double b2 = -0.0066;
        double b3 = -0.0375;
        double b4 = 0.0636;
        double b5 = -0.0144;
        double k = 0.0162;

        double Aa1 = 2.070 * Math.pow(10, -5);

        double Aa2 = -6.370 * Math.pow(10, -10);
        double Aa3 = 3.989 * Math.pow(10, -15);

        double Bb1 = 0.03426;

        double Bb2 = 0.0004464;
        double Bb3 = 0.4215;
        double Bb4 = -0.003107;

        double c0 = 0.6766097;
        double c1 = 0.0200564;
        double c2 = 0.0001104259;
        double c3 = -0.00000069698;
        double c4 = 0.0000000010031;

        double temp = 1.00024 * t;
        double R = c / 42.914;

        double rt = c0 + c1 * temp + c2 * temp * temp + c3 * temp * temp * temp + c4 * temp * temp * temp * temp;
        double alpha = (Aa1 * p + Aa2 * p * p + Aa3 * p * p * p) / (1 + Bb1 * temp + Bb2 * temp * temp + Bb3 * R + Bb4 * temp * R);
        double Rt = R / (rt * (1 + alpha));

        double S = a0 + a1 * Math.pow(Rt, 0.5) + a2 * Rt + a3 * Math.pow(Rt, 1.5) + a4 * Rt * Rt + a5 * Math.pow(Rt, 2.5) + (temp - 15) * (b0 + b1 * Math.pow(Rt, 0.5) + b2 * Rt + b3 * Math.pow(Rt, 1.5) + b4 * Rt * Rt + b5 * Math.pow(Rt, 2.5)) / (1 + k * (temp - 15));

        return S;
    }

    /**
     * Conductivity calculated from frequency and temp
     *
     * @param frek Frequency in kHz
     * @param temp Temperature in °C
     * @return
     */
    public static double condutctivity(double frek, double temp) {

        double g = -4.10731453e+000;
        double h = 5.02267656e-001;
        double i = -1.63378659e-004;
        double j = 3.70269818e-005;
        double t = temp; // 0°C
        double p = 0;

        double Pcor = -9.5700e-008;
        double Tcor = 3.2500e-006;

        System.out.println("f: " + frek + " kHz");
        System.out.println("g: " + g);
        System.out.println("h: " + h);
        System.out.println("i: " + i);
        System.out.println("j: " + new DecimalFormat("##.#######################").format(j));
        // double prev = (g + (h * Math.pow(f, 2)) + (i * Math.pow(f, 3)) + (j * Math.pow(f, 4))) / 10 * (1 + d * t + e * p);
        double prev = (g + h * Math.pow(frek, 2) + i * Math.pow(frek, 3) + j * Math.pow(frek, 4)) / 10 * (1 + Tcor * temp + Pcor * p);
        return prev;
    }

}
