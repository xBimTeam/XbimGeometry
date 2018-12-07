namespace XbimRegression
{
    class Program
    {
        private static void Main(string[] args)
        {
            // ContextTesting is a class that has been temporarily created to test multiple files
            // ContextTesting.Run();
            // return;

            var arguments = new Params(args);
            if (!arguments.IsValid)
                return;
            var processor = new BatchProcessor(arguments);
            processor.Run();
        }
    }
}
