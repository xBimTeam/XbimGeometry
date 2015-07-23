
namespace XbimRegression
{
    class Program
    {
        private static void Main(string[] args)
        {
            {
                Params arguments = new Params(args);


                if (arguments.IsValid)
                {
                    BatchProcessor processor = new BatchProcessor(arguments);
                    processor.Run();
                }
                arguments = null;
            }
        }


    }
}
