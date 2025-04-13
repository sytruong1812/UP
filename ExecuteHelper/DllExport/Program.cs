using System;
using System.Diagnostics;
using System.Runtime.InteropServices;
using System.Collections.Generic;
using System.Text;
using System.Reflection;
using System.Runtime.CompilerServices;
using System.Linq;
using System.IO;
using System.Web;

namespace ExportDll
{
    enum ParserState
    {
        Normal,
        ClassDeclaration,
        Class,
        MethodDeclaration,
        Method,
        AddExportAttribute,
    }
    class Program
    {
        static bool verbose = false;
        static List<string> methods;
        static Dictionary<System.Runtime.InteropServices.CallingConvention, string> diccc = new Dictionary<CallingConvention, string>();
        static Program()
        {
            diccc[System.Runtime.InteropServices.CallingConvention.Cdecl] = typeof(CallConvCdecl).FullName;
            diccc[System.Runtime.InteropServices.CallingConvention.FastCall] = typeof(CallConvFastcall).FullName;
            diccc[System.Runtime.InteropServices.CallingConvention.StdCall] = typeof(CallConvStdcall).FullName;
            diccc[System.Runtime.InteropServices.CallingConvention.ThisCall] = typeof(CallConvThiscall).FullName;
            diccc[System.Runtime.InteropServices.CallingConvention.Winapi] = typeof(CallConvStdcall).FullName;
        }

        static void Log(bool forced, string message, params object[] param)
        {
            if (forced || verbose)
            {
                Console.WriteLine(message, param);
            }
        }

        static void Log(string message, params object[] param)
        {
            Log(false, message, param);
        }

        static void Load()
        {
            Log(true, "In Load()");
            var dic = new Dictionary<string, Dictionary<string, KeyValuePair<string, string>>>();

            string filePath = (string)AppDomain.CurrentDomain.GetData("filePath");
            Log(true, $"GetData() -> file path: {filePath}");

            // Retrieve the list of method names
            List<string> methodNames = (List<string>)AppDomain.CurrentDomain.GetData("methodNames");

            Assembly assembly = Assembly.ReflectionOnlyLoadFrom(filePath);
            Type[] types = assembly.GetTypes();
            int exportsCount = 0;

            foreach (Type type in types)
            {
                Log(true, "Loop type...");

                // Iterate over each method name in the methodNames list
                foreach (string methodName in methodNames)
                {
                    MethodInfo mi = type.GetMethod(methodName, BindingFlags.Public | BindingFlags.Static);
                    if (mi != null)
                    {
                        Log(true, $"Found method: {methodName} in type: {type.FullName}");
                        if (!dic.ContainsKey(type.FullName))
                        {
                            dic[type.FullName] = new Dictionary<string, KeyValuePair<string, string>>();
                        }
                        dic[type.FullName][mi.Name] = new KeyValuePair<string, string>(mi.Name, typeof(CallConvStdcall).FullName);
                        exportsCount++;
                    }
                }
            }

            Log(true, "Break loop");
            AppDomain.CurrentDomain.SetData("exportsCount", exportsCount);
            AppDomain.CurrentDomain.SetData("dic", dic);
        }

        static Assembly CurrentDomain_ReflectionOnlyAssemblyResolve(object sender, ResolveEventArgs args)
        {
            return Assembly.ReflectionOnlyLoad(args.Name);
        }

        static int Main(string[] args)
        {
            try
            {
                if (args.Length < 1)
                {
                    Log(true, "DllExport.exe full_path_to_dll /method [/release|/debug] [/anycpu|/x64] [/verbose]");
                    Log(true, "Example: DllExport.exe ../../File.dll /method \"Method\" /release /anycpu /verbose");
                    Log(true, "Example: DllExport.exe ../../File.dll /method \"Method1 Method2 Method3\" /x64 /verbose");
                    return 1;
                }
                bool debug = false;
                List<string> listArgs = new List<string>(args);
                string filePath = listArgs.ElementAt(0); listArgs.RemoveAt(0);

                int mth = listArgs.FindIndex(new Predicate<string>(x => x.ToLower().Contains("/method")));
                if (mth > -1)
                {
                    string strMethod = listArgs.ElementAt(mth + 1);
                    methods = new List<string>(strMethod.Split(' '));
                    listArgs.RemoveAt(mth);
                    listArgs.RemoveAt(mth);
                }

                int deb = listArgs.FindIndex(new Predicate<string>(x => x.ToLower().Contains("/debug")));
                if (deb > -1)
                {
                    debug = true;
                }
                else
                {
                    int rel = listArgs.FindIndex(new Predicate<string>(x => x.ToLower().Contains("/release")));
                    if (rel > -1)
                    {
                        listArgs.RemoveAt(rel);
                        listArgs.Add("/optimize");
                    }
                }
                int any = listArgs.FindIndex(new Predicate<string>(x => x.ToLower().Contains("/anycpu")));
                if (any > -1)
                {
                    listArgs.RemoveAt(any);
                }
                else
                {
                    int x64 = listArgs.FindIndex(new Predicate<string>(x => x.ToLower().Contains("/x64")));
                    if (x64 > -1)
                    {
                        listArgs.Add("/PE64");
                    }
                }
                int verb = listArgs.FindIndex(new Predicate<string>(x => x.ToLower().Contains("/verbose")));
                if (verb > -1)
                {
                    verbose = true;
                    listArgs.RemoveAt(verb);
                }

                string dirPath = System.IO.Path.GetDirectoryName(filePath);
                if (dirPath == string.Empty)
                {
                    Log(true, "Full path needed!");
                    return 1;
                }
                string extName = System.IO.Path.GetExtension(filePath);
                if (extName != ".dll")
                {
                    Log(true, "Target should be dll!");
                    return 1;
                }

                Log(true, "Begin...");
                AppDomain domain = AppDomain.CreateDomain("ReflectionOnly");
                domain.ReflectionOnlyAssemblyResolve += new ResolveEventHandler(CurrentDomain_ReflectionOnlyAssemblyResolve);
                domain.SetData("filePath", filePath);
                domain.SetData("methodNames", methods);
                domain.DoCallBack(new CrossAppDomainDelegate(Program.Load));

                var dic = (Dictionary<string, Dictionary<string, KeyValuePair<string, string>>>)domain.GetData("dic");
                int exportsCount = (int)domain.GetData("exportsCount");
                AppDomain.Unload(domain);
                if (exportsCount > 0)
                {
                    Log(true, $"Number of Exports function = {exportsCount}");
                    int exportPos = 1;
                    string fileName = System.IO.Path.GetFileNameWithoutExtension(filePath);
                    System.IO.Directory.SetCurrentDirectory(dirPath);

                    Process proc = new Process();
                    string arguments = string.Format("/nobar{1}/out:{0}.il {0}.dll", fileName, debug ? " /linenum " : " ");
                    Log("Deassebly file with arguments '{0}'", arguments);

                    System.Diagnostics.ProcessStartInfo info = new ProcessStartInfo(DllExport.Properties.Settings.Default.ildasm, arguments);
                    info.UseShellExecute = false;
                    info.CreateNoWindow = false;
                    info.RedirectStandardOutput = true;
                    proc.StartInfo = info;
                    proc.Start();
                    proc.WaitForExit();
                    Log(proc.ExitCode != 0, proc.StandardOutput.ReadToEnd());
                    if (proc.ExitCode != 0)
                    {
                        return proc.ExitCode;
                    }

                    List<string> wholeilFile = new List<string>();
                    System.IO.StreamReader sr = System.IO.File.OpenText(System.IO.Path.Combine(dirPath, fileName + ".il"));
                    string methodDeclaration = "";
                    string methodName = "";
                    string classdeClaration = "";
                    string methodBefore = "";
                    string methodAfter = "";
                    int methodPos = 0;
                    Stack<string> classNames = new Stack<string>();
                    List<string> externAssembly = new List<string>();
                    ParserState state = ParserState.Normal;
                    while (!sr.EndOfStream)
                    {
                        string line = sr.ReadLine();
                        string trimedLine = line.Trim();
                        bool addLine = true;
                        switch (state)
                        {
                            case ParserState.Normal:
                                if (trimedLine.StartsWith(".class"))
                                {
                                    state = ParserState.ClassDeclaration;
                                    addLine = false;
                                    classdeClaration = trimedLine;
                                }
                                break;
                            case ParserState.ClassDeclaration:
                                if (trimedLine.StartsWith("{"))
                                {
                                    state = ParserState.Class;
                                    string className = "";
                                    System.Text.RegularExpressions.Regex reg = new System.Text.RegularExpressions.Regex(@".+\s+([^\s]+) extends \[.*");
                                    System.Text.RegularExpressions.Match m = reg.Match(classdeClaration);
                                    if (m.Groups.Count > 1)
                                    {
                                        className = m.Groups[1].Value;
                                    }
                                    className = className.Replace("'", "");
                                    if (classNames.Count > 0)
                                    {
                                        className = classNames.Peek() + "+" + className;
                                    }
                                    classNames.Push(className);
                                    Log("Found class: " + className);
                                    wholeilFile.Add(classdeClaration);
                                }
                                else
                                {
                                    classdeClaration += " " + trimedLine;
                                    addLine = false;
                                }
                                break;
                            case ParserState.Class:
                                if (trimedLine.StartsWith(".class"))
                                {
                                    state = ParserState.ClassDeclaration;
                                    addLine = false;
                                    classdeClaration = trimedLine;
                                }
                                else if (trimedLine.StartsWith(".method"))
                                {
                                    if (dic.ContainsKey(classNames.Peek()))
                                    {
                                        methodDeclaration = trimedLine;
                                        addLine = false;
                                        state = ParserState.MethodDeclaration;
                                    }
                                }
                                else if (trimedLine.StartsWith("} // end of class"))
                                {
                                    classNames.Pop();
                                    if (classNames.Count > 0)
                                    {
                                        state = ParserState.Class;
                                    }
                                    else
                                    {
                                        state = ParserState.Normal;
                                    }
                                }
                                break;
                            case ParserState.MethodDeclaration:
                                if (trimedLine.StartsWith("{"))
                                {
                                    System.Text.RegularExpressions.Regex reg = new System.Text.RegularExpressions.Regex(@"(?<before>[^\(]+(\(\s[^\)]+\))*\s)(?<method>[^\(]+)(?<after>\(.*)");
                                    System.Text.RegularExpressions.Match m = reg.Match(methodDeclaration);
                                    if (m.Groups.Count > 3)
                                    {
                                        methodBefore = m.Groups["before"].Value;
                                        methodAfter = m.Groups["after"].Value;
                                        methodName = m.Groups["method"].Value;
                                    }
                                    Log("Found method: " + methodName);
                                    if (dic[classNames.Peek()].ContainsKey(methodName))
                                    {
                                        methodPos = wholeilFile.Count;
                                        state = ParserState.AddExportAttribute;
                                    }
                                    else
                                    {
                                        wholeilFile.Add(methodDeclaration);
                                        state = ParserState.Method;
                                        methodPos = 0;
                                    }
                                }
                                else
                                {
                                    methodDeclaration += " " + trimedLine;
                                    addLine = false;
                                }
                                break;
                            case ParserState.Method:
                                if (trimedLine.StartsWith("} // end of method"))
                                {
                                    state = ParserState.Class;
                                }
                                break;
                            case ParserState.AddExportAttribute:
                                if (trimedLine.StartsWith(".custom") || trimedLine.StartsWith("// Code"))
                                {
                                    KeyValuePair<string, string> attr = dic[classNames.Peek()][methodName];
                                    if (methodBefore.Contains("marshal( "))
                                    {
                                        int pos = methodBefore.IndexOf("marshal( ");
                                        methodBefore = methodBefore.Insert(pos, "modopt([mscorlib]" + attr.Value + ") ");
                                        methodDeclaration = methodBefore + methodName + methodAfter;
                                    }
                                    else
                                    {
                                        Log("\tChanging calling convention: " + attr.Value);
                                    }

                                    if (methodPos != 0)
                                    {
                                        wholeilFile.Insert(methodPos, methodDeclaration);
                                    }
                                    if (methodName == "DllMain")
                                    {
                                        wholeilFile.Add(" .entrypoint");
                                    }
                                    wholeilFile.Add(string.Format(".export [{0}] as {1}", exportPos, dic[classNames.Peek()][methodName].Key));

                                    Log("\tAdding .vtentry:{0} .export:{1}", exportPos, dic[classNames.Peek()][methodName].Key);
                                    exportPos++;
                                    state = ParserState.Method;
                                }
                                else
                                {
                                    addLine = false;
                                }
                                break;
                        }
                        if (addLine)
                        {
                            wholeilFile.Add(line);
                        }
                    }
                    sr.Close();
                    System.IO.StreamWriter sw = System.IO.File.CreateText(System.IO.Path.Combine(dirPath, fileName + ".il"));
                    foreach (string line in wholeilFile)
                    {
                        sw.WriteLine(line);
                    }
                    sw.Close();
                    string res = fileName + ".res";
                    if (System.IO.File.Exists(fileName + ".res"))
                    {
                        res = " /resource=" + res;
                    }
                    else
                    {
                        res = "";
                    }
                    proc = new Process();
                    arguments = string.Format("/nologo /quiet /out:{0}.dll {0}.il /DLL{1} {2}", fileName, res, string.Join(" ", listArgs.ToArray()));
                    Log("Compiling file with arguments '{0}'", arguments);
                    info = new ProcessStartInfo(DllExport.Properties.Settings.Default.ilasm, arguments);
                    info.UseShellExecute = false;
                    info.CreateNoWindow = false;
                    info.RedirectStandardOutput = true;
                    proc.StartInfo = info;
                    proc.Start();
                    proc.WaitForExit();
                    Log(proc.ExitCode != 0, proc.StandardOutput.ReadToEnd());
                    if (proc.ExitCode != 0)
                    {
                        return proc.ExitCode;
                    }
                }
            }
            catch (Exception ex)
            {
                Console.WriteLine(ex.Message);
                return -1;
            }
            return 0;
        }
    }
}
