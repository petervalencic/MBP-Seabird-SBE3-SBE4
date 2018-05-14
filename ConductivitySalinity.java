import java.text.DecimalFormat;

/**
 * Simple example for SBE-4 seabird conductiviry sensor
 *
 * @author Peter Valencic
 */
public class ConductivitySalinity {

   public static void main(String[] args) {
        double slanost = SeaWater.salinity(4.26923 * 10, 15, 0);
        System.out.println("Slanost iz prevodnosti instrumenta " + 4.26923 + ": slanost: " + slanost);
        double slanost2 = SeaWater.salinity(4.26927 * 10, 15, 0);
        System.out.println("Slanost iz prevodnosti kopeli " + 4.26927 + ": slanost:  " + slanost2);
        System.out.println("Residual: " + (slanost - slanost2));
        System.out.println("======================================================");
        double slanost_s1 = slanost(15, 4.26923 * 10, 0);
        double slanost_s2 = slanost(15, 4.26927 * 10, 0);
        System.out.println("Druga funkcija, slanost iz prevodnosti instrumenta " + 4.26923 + ": slanost: " + slanost_s1);
        System.out.println("Druga funkcija, slanost prevodnosti kopeli " + 4.26927 + ": slanost:  " + slanost_s2);
        System.out.println("Residual: " + (slanost_s1 - slanost_s2));
        System.out.println("======================================================");
        double slanost_a1 = calcctd(15, 4.26923 * 10, 0);
        double slanost_a2 = calcctd(15, 4.26927 * 10, 0);
        System.out.println("Druga funkcija, slanost iz prevodnosti instrumenta " + 4.26923 + ": slanost: " + slanost_a1);
        System.out.println("Druga funkcija, slanost prevodnosti kopeli " + 4.26927 + ": slanost:  " + slanost_a2);
        System.out.println("Residual: " + (slanost_a1 - slanost_a2));

        double A1 = 2.070E-5; //Math.pow(10,-5);
        double A2 = -6.370E-10; //*Math.pow(10,-10);
        double A3 = 3.989E-15; //*Math.pow(10,-15);
        System.out.println("A1 :" + A1);
        System.out.println("A2 :" + A2);
        System.out.println("A3 :" + A3);
        A1 = 2.070 * Math.pow(10, -5);
        A2 = -6.370 * Math.pow(10, -10);
        A3 = 3.989 * Math.pow(10, -15);
        System.out.println("A1 :" + A1);
        System.out.println("A2 :" + A2);
        System.out.println("A3 :" + A3);

    }

    public static double calcctd(double Temperatura, double Conductivity, double Pressure) {

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

        double A1 = 2.070E-5; //Math.pow(10,-5);
        double A2 = -6.370E-10; //*Math.pow(10,-10);
        double A3 = 3.989E-15; //*Math.pow(10,-15);
        double B1 = 0.03426;
        double B2 = 0.0004464;
        double B3 = 0.4215;
        double B4 = -0.003107;

        double c0 = 0.6766097;
        double c1 = 0.0200564;
        double c2 = 0.0001104259;
        double c3 = -0.00000069698;
        double c4 = 0.0000000010031;

        double T = Temperatura;
        double R = Conductivity / 42.914;
        double P = Pressure;

        T = 1.00024 * T;

        double rt = c0 + c1 * T + c2 * T * T + c3 * T * T * T + c4 * T * T * T * T;
        double alpha = (A1 * P + A2 * P * P + A3 * P * P * P) / (1 + B1 * T + B2 * T * T + B3 * R + B4 * T * R);
        double Rt = R / (rt * (1 + alpha));

        double S = a0 + a1 * Math.pow(Rt, 0.5) + a2 * Rt + a3 * Math.pow(Rt, 1.5) + a4 * Rt * Rt + a5 * Math.pow(Rt, 2.5) + (T - 15) * (b0 + b1 * Math.pow(Rt, 0.5) + b2 * Rt + b3 * Math.pow(Rt, 1.5) + b4 * Rt * Rt + b5 * Math.pow(Rt, 2.5)) / (1 + k * (T - 15));

        return S;

    }

    public static double slanost(double t, double c, double p) {
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

}
