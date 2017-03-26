namespace XbimRegression
{
    internal class Program
    {
        private static void Main(string[] args)
        {
            var arguments = new Params(args);
            if (!arguments.IsValid)
                return;
            var processor = new BatchProcessor(arguments);
            processor.Run();
        }
    }
}
