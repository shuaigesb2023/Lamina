#include <string>
#include <utility>
#include <vector>
namespace templatestdftr {
    /*
    ������ģ�廯
    ������Ҫ֧�� + - / % * = == 
    */
    template<class TP = unsigned long long>
    TP unllgcd(TP a, TP b) {//�����Լ����Ĭ��a>=b
        if (b == TP(0)) return a;
        return unllgcd(b, a % b);
    }
    std::pair<unsigned long long, unsigned long long> jiance(std::vector<short>& n) {
        //���ѭ�����֣�����ѭ���Ŀ�ʼ�ͽ���������ұգ�
        unsigned long long h1[10] = {};//ʹ�ù�ϣ����¼���ֳ��ֵĴ���
        unsigned long long h2[10] = {};

        for (short& i: n) h1[i]++;//��һ�ֱ�����¼ÿ�����ֵ��ܳ��ִ�����ʹ�õ�������
        bool flag = true;

        for (unsigned long long i = 0; i < n.size(); i++) {//�ڶ��ֱ���
            flag = true;
            for (int j = 0; j < 10; j++) {
                if ((h1[j] - h2[j]) & 1) {
                    flag = false;
                    break;
                }
            }
            if (flag) {
                unsigned long long i1 = i + 1, i2 = n.size();
                unsigned long long ti1, ti2;
                bool ok = true;
                while (!((i2 - i1) & 1) and ok) {
                    //����Ƿ�ѭ��
                    //ʹ��˫ָ��
                    //��θĵıȽ϶಻дСע����
                    //ԭ��Ϊ���Ͻ�ѭ����Χ����ֱ����ѭ��Ϊֹ�����һ�ֲ�ѭ�������ڶ��ֱ���
                    ti1 = i1;
                    ti2 = i1 + ((i2 - i1) >> 1);
                    while (ti2 < i2) {
                        if (n[ti1] != n[ti2]) {
                            ok = false;
                            break;
                        }
                        ti1++;
                        ti2++;
                    }
                    if (ok) {
                        i2 = i1 + ((i2 - i1) >> 1);
                    } else if (i2 == n.size()) {
                        break;
                    } else {
                        return {i1, i2 - 1};
                    }
                }
                if (ok) {//�������ѭ������
                    return {i1, i2 - 1};
                }
            }
            h2[n[i]]++;
        }
        return {n.size() - 1, n.size() - 1};
    }
    template<class TP = unsigned long long>
    std::pair<TP, TP> float_to_rational(const std::string num) {
        //����ֵ��һ��Ϊ���ӵڶ���Ϊ��ĸ
        TP up = TP(0), down = TP(1);//up���ӣ�down��ĸ
        unsigned long long i = 0;
        while (i < num.size() and num[i] != '.') {//������������
            up = up * TP(10) + TP(num[i] - '0');
            i++;
        }
        if (i == num.size()) return {up, down};//����С����ֱ�ӷ���
        i++;
        std::vector<short> n;//ʹ��vector������ʱ�洢С��λ
        while (i < num.size() and num[i] != '.') {
            n.emplace_back(num[i] - '0');
            i++;
        }
        if (i == num.size()) {//����Ƿ�Ϊѭ��С��
            //������ѭ��С��
            //ֱ�����ӵ�ĩβ
            for (short& i: n) {
                up = up * TP(10) + TP(i);
                down = down * TP(10);
            }
        } else {
            //����ѭ��С��
            //�ȼ��ѭ�����֣��ٽ���ѭ������ֱ�����ӣ��ټӣ�ѭ������/999...9��ѭ�����ֳ��ȸ�9��/pow��10��len����ѭ�����֣�����
            std::pair<unsigned long long, unsigned long long> xun = jiance(n);

            //�ñ�����ʱ��¼ѭ������
            TP xup = TP(0), xdown = TP(0);
            for (unsigned long long i = xun.first; i <= xun.second; i++) {
                xup = xup * TP(10) + TP(n[i]);
                xdown = xdown * TP(10) + TP(9);
            }

            //����ѭ����������
            for (unsigned long long i = 0; i < xun.first; i++) {
                up = up * TP(10) + TP(n[i]);
                down = down * TP(10);
            }

            //ͨ��
            up = up * xdown;
            down = down * xdown;

            //���
            up = up + xup;
        }
        //Լ�ֻ���ʹ��gcd
        TP g = unllgcd<TP>(up, down);
        up = up / g;
        down = down / g;

        //����
        return {up, down};
    }
}// namespace templatestdftr