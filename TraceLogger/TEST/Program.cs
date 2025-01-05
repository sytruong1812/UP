using System;
using System.Threading;
using TraceLogger;

class TEST
{
    static void Main(string[] args)
    {
        Console.WriteLine("=== Testing Logger in Main Thread ===");
        Logger.Log("This is a test message from the Main thread.");

        Console.WriteLine("\n=== Testing Logger in Multiple Threads ===");
        TestMultithreading();


        Console.WriteLine("=== Testing Trace ===");

        Logger.Trace("Starting Main...");

        TestMethod("Hello", 42, true);

        Logger.Trace("Exiting Main...");

    }

    static void TestMultithreading()
    {
        int numberOfThreads = 5; 
        Thread[] threads = new Thread[numberOfThreads];

        for (int i = 0; i < numberOfThreads; i++)
        {
            int threadNumber = i + 1;
            threads[i] = new Thread(() =>
            {
                Logger.Log($"Log message from Thread #{threadNumber}, Current ThreadId: {Thread.CurrentThread.ManagedThreadId}");
                Thread.Sleep(100); 
                Logger.Log_v2($"Log_v2 message from Thread #{threadNumber}, Current ThreadId: {Thread.CurrentThread.ManagedThreadId}");
            });
        }

        foreach (var thread in threads)
        {
            thread.Start();
        }
        foreach (var thread in threads)
        {
            thread.Join();
        }
    }

    static int TestMethod(string str, int num, bool flag)
    {
        Logger.Trace_In(nameof(TestMethod), new object[] { str, num, flag });

        // Do some work
        int result = num * 2;

        Logger.Trace_Out(nameof(TestMethod), result);

        return result;
    }
}
