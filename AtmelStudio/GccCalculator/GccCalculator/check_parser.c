/******************************************************************

Introduction:
mathparser is a simple c++ program to parse math expressions.

The program is a modified version of math expression parser
presented in the book : "C++ The Complete Reference" by H.Schildt.

It supports operators: + - * / ^ ( )

It supports math functions : SIN, COS, TAN, ASIN, ACOS, ATAN, SINH,
COSH, TANH, ASINH, ACOSH, ATANH, LN, LOG, EXP, SQRT, SQR, ROUND, INT.

It supports variables A to Z.

Sample:
25 * 3 + 1.5*(-2 ^ 4 * log(30) / 3)
x = 3
y = 4
r = sqrt(x ^ 2 + y ^ 2)
t = atan(y / x)

mathparser version 1.0 by Hamid Soltani. (gmail: hsoltanim)
Last modified: Aug. 2016.

*******************************************************************/

#include<stdio.h>
#include<math.h>
#include<string.h>
#include<stdlib.h>
#include<ctype.h>
#define PI 3.14159265358979323846
#define NUMVARS 26
enum types { DELIMITER = 1, VARIABLE, NUMBER, FUNCTION };

char *exp_ptr; // points to the expression
char token[26]; // holds current token
char tok_type; // holds token's type
double vars[NUMVARS]; // holds variable's values
double eval_exp1(double result);
double eval_exp2(double result);
double eval_exp3(double result);
double eval_exp4(double result);
double eval_exp5(double result);
double eval_exp6(double result);
void get_token();
double eval_exp(char *exp);
char errormsg[20];
char rans[18];
// Parser constructor.
void parser()
{
	int i;
	exp_ptr = NULL;
	for (i = 0; i < NUMVARS; i++)
		vars[i] = 0.0;
	errormsg[0] = '\0';
}
// Parser entry point.
double eval_exp(char *exp)
{
	errormsg[0] = '\0';
	exp_ptr = exp;
	get_token();
	if (!*token)
	{
		strcpy(errormsg, "Error"); // no expression present
		return (double)0;
	}
	double result=eval_exp1(result);
	if (*token) // last token must be null
		strcpy(errormsg, "Error");
	return result;
}
// Process an assignment.
double eval_exp1(double result)
{
	int slot;
	char temp_token[80];
	if (tok_type == VARIABLE)
	{
		// save old token
		char *t_ptr = exp_ptr;
		strcpy(temp_token, token);
		// compute the index of the variable
		slot = *token - 'A';
		get_token();
		if (*token != '=')
		{
			exp_ptr = t_ptr; // return current token
			strcpy(token, temp_token); // restore old token
			tok_type = VARIABLE;
		}
		else {
			get_token(); // get next part of exp
			result=eval_exp2(result);
			vars[slot] = result;
			return result;
		}
	}
	result=eval_exp2(result);
	return result;
}
// Add or subtract two terms.
double eval_exp2(double result)
{
	register char op;
	double temp=0;
	result=eval_exp3(result);
	while ((op = *token) == '+' || op == '-')
	{
		get_token();
		temp=eval_exp3(temp);
		switch (op)
		{
		case '-':
			result = result - temp;
			break;
		case '+':
			result = result + temp;
			break;
		}
	}
	return result;
}
// Multiply or divide two factors.
double eval_exp3(double result)
{
	register char op;
	double temp=0;
	result=eval_exp4(result);
	while ((op = *token) == 'x' || op == '/')
	{
		get_token();
		temp=eval_exp4(temp);
		switch (op)
		{
		case 'x':
			result = result * temp;
			break;
		case '/':
			result = result / temp;
			break;
		}
	}
	return result;
}
// Process an exponent.
double eval_exp4(double result)
{
	double temp=0;
	result=eval_exp5(result);
	while (*token == '^')
	{
		get_token();
		temp=eval_exp5(temp);
		result = pow(result, temp);
	}
	return result;
}
// Evaluate a unary + or -.
double eval_exp5(double result)
{
	register char op;
	op = 0;
	if ((tok_type == DELIMITER) && (*token == '+' || *token == '-'))
	{
		op = *token;
		get_token();
	}
	result=eval_exp6(result);
	if (op == '-')
		result = -result;
    return result;
}
// Process a function, a parenthesized expression, a value or a variable
double eval_exp6(double result)
{
	int isfunc = (tok_type == FUNCTION);
	char temp_token[80];
	if (isfunc)
	{
		strcpy(temp_token, token);
		get_token();
	}
	if ((*token == '('))
	{
		get_token();
		result=eval_exp2(result);
		if (*token != ')')
			strcpy(errormsg, "Error");
		if (isfunc)
		{
			if (!strcmp(temp_token, "SIN"))
				result = sin(PI / 180 * result);
			else if (!strcmp(temp_token, "COS"))
				result = cos(PI / 180 * result);
			else if (!strcmp(temp_token, "TAN"))
				result = tan(PI / 180 * result);
			else if (!strcmp(temp_token, "ASIN"))
				result = 180 / PI*asin(result);
			else if (!strcmp(temp_token, "ACOS"))
				result = 180 / PI*acos(result);
			else if (!strcmp(temp_token, "ATAN"))
				result = 180 / PI*atan(result);
			else if (!strcmp(temp_token, "SINH"))
				result = sinh(result);
			else if (!strcmp(temp_token, "COSH"))
				result = cosh(result);
			else if (!strcmp(temp_token, "TANH"))
				result = tanh(result);
			else if (!strcmp(temp_token, "LN"))
				result = log(result);
			else if (!strcmp(temp_token, "LOG"))
				result = log10(result);
			else if (!strcmp(temp_token, "E"))
				result = exp(result);
			else if (!strcmp(temp_token, "SQRT"))
				result = sqrt(result);
			else if (!strcmp(temp_token, "SQR"))
				result = result*result;
			else if (!strcmp(temp_token, "ROUND"))
				result = round(result);
			else if (!strcmp(temp_token, "INT"))
				result = floor(result);
			else
				strcpy(errormsg, "Error");
		}
		get_token();
	}
	else
		switch (tok_type)
		{
		case VARIABLE:
			result = vars[*token - 'A'];
			get_token();
			return result;
		case NUMBER:
			result = atof(token);
			get_token();
			return result;
		default:
			strcpy(errormsg, "Syntax_Error");
		}
    return result;
}
// Obtain the next token.
void get_token()
{
	register char *temp;
	tok_type = 0;
	temp = token;
	*temp = '\0';
	if (!*exp_ptr)  // at end of expression
		return;
	while (isspace(*exp_ptr))  // skip over white space
		++exp_ptr;
	if (strchr("+-x/%^=()", *exp_ptr))
	{
		tok_type = DELIMITER;
		*temp++ = *exp_ptr++;  // advance to next char
	}
	else if (isalpha(*exp_ptr))
	{
		while (!strchr(" +-/x%^=()\t\r", *exp_ptr) && (*exp_ptr))
			*temp++ = toupper(*exp_ptr++);
		while (isspace(*exp_ptr))  // skip over white space
			++exp_ptr;
		tok_type = (*exp_ptr == '(') ? FUNCTION : VARIABLE;
	}
	else if (isdigit(*exp_ptr) || *exp_ptr == '.')
	{
		while (!strchr(" +-/x%^=()\t\r", *exp_ptr) && (*exp_ptr))
			*temp++ = toupper(*exp_ptr++);
		tok_type = NUMBER;
	}
	*temp = '\0';
	if ((tok_type == VARIABLE) && (token[1]))
		strcpy(errormsg, "Error");
}

/*int main()
{
	char expstr[256];
	printf("Math expression parser. Enter a blank line to stop.\n\n");

	{
		printf("Enter expression: ");
		gets(expstr);
		double ans = eval_exp(expstr);
		if (*errormsg)
        {
            printf("%s\n\n",errormsg);
        }
		else
			printf("%.8f",ans);
	}
	return 0;
}*/

int n_tu(int number, int count)
{
    int result = 1;
    while(count-- > 0)
        result *= number;

    return result;
}

/*** Convert float to string ***/
void double_to_string(double f, char r[])
{
    long long int length, length2, i, number, position, sign;
    double number2;

    sign = -1;   // -1 == positive number
    if (f < 0)
    {
        sign = '-';
        f *= -1;
    }

    number2 = f;
    number = f;
    length = 0;  // Size of decimal part
    length2 = 0; // Size of tenth

    while( (number2 - (double)number) != 0.0 && !((number2 - (double)number) < 0.0) )
    {
         number2 = f * (n_tu(10.0, length2 + 1));
         number = number2;

         length2++;
    }

    for (length = (f > 1) ? 0 : 1; f > 1; length++)
        f /= 10;

    position = length;
    length = length + 1 + length2;
    number = number2;
    if (sign == '-')
    {
        length++;
        position++;
    }

    for (i = length; i >= 0 ; i--)
    {
        if (i == (length))
            r[i] = '\0';
        else if(i == (position))
            r[i] = '.';
        else if(sign == '-' && i == 0)
            r[i] = '-';
        else
        {
            r[i] = (number % 10) + '0';
            number /=10;
        }
    }
}
void reverse(char str[], int len)
{
    int i = 0, j = len - 1, temp;
    while (i < j) {
        temp = str[i];
        str[i] = str[j];
        str[j] = temp;
        i++;
        j--;
    }
}
int intToStr(int x, char str[], int d)
{
    int i = 0;
    while (x) {
        str[i++] = (x % 10) + '0';
        x = x / 10;
    }

    // If number of digits required is more, then
    // add 0s at the beginning
    while (i < d)
        str[i++] = '0';

    reverse(str, i);
    str[i] = '\0';
    return i;
}
void ftoa(double n, char* res, int afterpoint)
{
    // Extract integer part
    int ipart = (int)n;

    // Extract floating part
    double fpart = n - (double)ipart;

    // convert integer part to string
    int i = intToStr(ipart, res, 0);

    // check for display option after point
    if (afterpoint != 0) {
        res[i] = '.'; // add dot

        // Get the value of fraction part upto given no.
        // of points after dot. The third parameter
        // is needed to handle cases like 233.007
        fpart = fpart * pow(10, afterpoint);

        intToStr((int)fpart, res + i + 1, afterpoint);
    }
}
double parse_exp(char *expstr)
{
    int i,len=strlen(expstr);
    for(i=0;i<len;i++)
    {
        if('a'<=expstr[i]&&expstr[i]<='z'&&expstr[i]!='x')expstr[i]=expstr[i]-32;
    }
    double ans = eval_exp(expstr);
    for(int i=0; i<20; i++) {
        rans[i] = '\0';
    }
    if (*errormsg)
    {
        /*int i,n=strlen(errormsg);
        for(i=0;i<n;i++)
        {
            rans[i]=errormsg[i];
            errormsg[i]='\0';
        }
        rans[n]='\0';*/
        ans=1e18;
    }
    else if(ans<0)
    {
        ans=ans-0.0005;
    }
	else
	{
		ans=ans+0.0005;
	}
    return ans;
}
